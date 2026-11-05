/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2026 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * tdksrc
 * ------
 * A standalone GStreamer bin that acts as a dual elementary-stream feeder.
 * Internally it wraps a pair of appsrc pushers:
 *   * video path  -> exposed on ghost pad "src_video"
 *   * audio path  -> exposed on ghost pad "src_audio"
 *
 * The bin advertises the custom "tdk://" URI scheme so playbin can select it
 * automatically. Both appsrc children (and their ghost pads) are wired up while
 * the instance is being constructed. Applications reach the underlying pushers
 * with gst_tdk_src_get_video_appsrc() / gst_tdk_src_get_audio_appsrc() and feed
 * raw ES buffers into them. No caps are declared here on purpose, letting the
 * downstream decodebin run typefind on whatever is pushed.
 */
#include "GstTdkSrc.h"

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>

struct _GstTdkSrcPrivate {
    gchar*      location;
    GstElement* videoAppsrc;
    GstElement* audioAppsrc;
    GstPad*     videoGhostPad;
    GstPad*     audioGhostPad;
};

enum TdkSrcProperty {
    TDK_PROP_NONE = 0,
    TDK_PROP_LOCATION
};

static void gst_tdk_src_uri_handler_init(gpointer gIface, gpointer ifaceData);

#define gst_tdk_src_parent_class parent_class
GST_DEBUG_CATEGORY_STATIC(gst_tdk_src_debug);
G_DEFINE_TYPE_WITH_CODE(GstTdkSrc, gst_tdk_src, GST_TYPE_BIN,
        G_ADD_PRIVATE(GstTdkSrc)
        G_IMPLEMENT_INTERFACE(GST_TYPE_URI_HANDLER, gst_tdk_src_uri_handler_init);
        GST_DEBUG_CATEGORY_INIT(gst_tdk_src_debug, "tdksrc", 0, "tdksrc dual appsrc feeder"););

static GstStaticPadTemplate kSrcPadTemplate =
        GST_STATIC_PAD_TEMPLATE("src_%u", GST_PAD_SRC, GST_PAD_SOMETIMES, GST_STATIC_CAPS_ANY);

static void gst_tdk_src_finalize(GObject*);
static void gst_tdk_src_set_property(GObject*, guint propertyID, const GValue*, GParamSpec*);
static void gst_tdk_src_get_property(GObject*, guint propertyID, GValue*, GParamSpec*);
static GstStateChangeReturn gst_tdk_src_change_state(GstElement*, GstStateChange);
static void gst_tdk_src_handle_message(GstBin*, GstMessage*);
static gboolean gst_tdk_src_ghost_query(GstPad*, GstObject*, GstQuery*);
static GstPad* gst_tdk_src_expose_pad(GstTdkSrc* self, GstElement* pusher, const gchar* ghostName);

/*
 * Instantiate a single appsrc pusher and park it inside the bin. The ghost pad
 * that surfaces it is NOT created here: it is added later (see
 * gst_tdk_src_expose_pad, called from change_state READY->PAUSED) so that the
 * pad-added signal fires only after uridecodebin has taken the source and
 * connected its pad-added handler. Adding the SOMETIMES pads in init() - before
 * the element is parented - means uridecodebin never observes them, concludes
 * the source produced nothing, and re-instantiates it (a second source-setup
 * that tears the sink down). The production primesrc likewise adds its pads
 * after source-setup (configure_src), not at construction.
 */
static GstElement* gst_tdk_src_build_stream(GstTdkSrc* self, const gchar* childName)
{
    GstElement* pusher = gst_element_factory_make("appsrc", childName);
    if (pusher == NULL) {
        GST_ERROR_OBJECT(self, "Unable to instantiate appsrc child '%s'", childName);
        return NULL;
    }

    /* Streaming, non-live, TIME formatted; caps deliberately omitted. */
    g_object_set(G_OBJECT(pusher),
            "stream-type",  GST_APP_STREAM_TYPE_STREAM,
            "format",       GST_FORMAT_TIME,
            "is-live",      FALSE,
            "block",        FALSE,
            "emit-signals", TRUE,
            NULL);

    gst_bin_add(GST_BIN(self), pusher);

    GST_INFO_OBJECT(self, "appsrc child '%s' created (pad exposed later)", childName);
    return pusher;
}

/*
 * Create the ghost pad for an already-added appsrc and expose it on the bin.
 * Wired exactly like primesrc's configure_src: a query function forwarding to
 * the appsrc target pad plus GST_PAD_FLAG_NEED_PARENT. Called from change_state
 * so uridecodebin observes the pad-added signal.
 */
static GstPad* gst_tdk_src_expose_pad(GstTdkSrc* self, GstElement* pusher, const gchar* ghostName)
{
    if (pusher == NULL)
        return NULL;

    GstPad* innerPad = gst_element_get_static_pad(pusher, "src");
    GstPad* ghost = gst_ghost_pad_new(ghostName, innerPad);
    gst_object_unref(innerPad);

    gst_pad_set_query_function(ghost, gst_tdk_src_ghost_query);
    gst_pad_set_active(ghost, TRUE);
    gst_element_add_pad(GST_ELEMENT(self), ghost);
    GST_OBJECT_FLAG_SET(ghost, GST_PAD_FLAG_NEED_PARENT);

    GST_INFO_OBJECT(self, "appsrc child now reachable on ghost pad '%s'", ghostName);
    return ghost;
}

static void gst_tdk_src_class_init(GstTdkSrcClass* klass)
{
    GObjectClass*    gobjectClass = G_OBJECT_CLASS(klass);
    GstElementClass* elementClass = GST_ELEMENT_CLASS(klass);
    GstBinClass*     binClass     = GST_BIN_CLASS(klass);

    gobjectClass->set_property = gst_tdk_src_set_property;
    gobjectClass->get_property = gst_tdk_src_get_property;
    gobjectClass->finalize     = gst_tdk_src_finalize;

    g_object_class_install_property(gobjectClass, TDK_PROP_LOCATION,
            g_param_spec_string("location", "Location", "URI/location to source from",
                NULL, (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

    gst_element_class_add_pad_template(elementClass, gst_static_pad_template_get(&kSrcPadTemplate));
    gst_element_class_set_metadata(elementClass,
            "tdksrc dual ES feeder",
            "Source/Bin",
            "Pushes separate video and audio elementary streams through two embedded appsrc",
            "RDK User <rdkuser@comcast.com>");

    elementClass->change_state = GST_DEBUG_FUNCPTR(gst_tdk_src_change_state);
    binClass->handle_message   = GST_DEBUG_FUNCPTR(gst_tdk_src_handle_message);
}

static void gst_tdk_src_init(GstTdkSrc* self)
{
    GstTdkSrcPrivate* priv = (GstTdkSrcPrivate*) gst_tdk_src_get_instance_private(self);
    self->priv = priv;

    priv->location      = NULL;
    priv->videoGhostPad = NULL;
    priv->audioGhostPad = NULL;

    /*
     * Create both appsrc pushers eagerly (so the accessors work as soon as
     * source-setup fires) but expose their ghost pads later, from change_state,
     * once uridecodebin is watching for pad-added.
     */
    priv->videoAppsrc = gst_tdk_src_build_stream(self, "videosrc");
    priv->audioAppsrc = gst_tdk_src_build_stream(self, "audiosrc");
}

static void gst_tdk_src_finalize(GObject* object)
{
    GstTdkSrcPrivate* priv = GST_TDK_SRC(object)->priv;

    if (priv->location != NULL) {
        g_free(priv->location);
        priv->location = NULL;
    }

    GST_CALL_PARENT(G_OBJECT_CLASS, finalize, (object));
}

static void gst_tdk_src_set_property(GObject* object, guint propId, const GValue* value, GParamSpec* pspec)
{
    GstTdkSrc* self = GST_TDK_SRC(object);

    if (propId == TDK_PROP_LOCATION) {
        gst_uri_handler_set_uri(GST_URI_HANDLER(self), g_value_get_string(value), NULL);
    } else {
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, propId, pspec);
    }
}

static void gst_tdk_src_get_property(GObject* object, guint propId, GValue* value, GParamSpec* pspec)
{
    GstTdkSrc* self = GST_TDK_SRC(object);

    if (propId == TDK_PROP_LOCATION) {
        GST_OBJECT_LOCK(self);
        g_value_set_string(value, self->priv->location);
        GST_OBJECT_UNLOCK(self);
    } else {
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, propId, pspec);
    }
}

static GstStateChangeReturn gst_tdk_src_change_state(GstElement* element, GstStateChange transition)
{
    GstTdkSrc* self = GST_TDK_SRC(element);
    GstTdkSrcPrivate* priv = self->priv;

    /*
     * Expose the ghost pads on the READY->PAUSED edge, BEFORE chaining up. By
     * now uridecodebin has created the source and connected its pad-added
     * handler, so it observes both pads and does not re-instantiate the source.
     * gst_element_no_more_pads() is issued afterwards to signal the pad set is
     * complete.
     */
    if (transition == GST_STATE_CHANGE_READY_TO_PAUSED) {
        if (priv->videoGhostPad == NULL)
            priv->videoGhostPad = gst_tdk_src_expose_pad(self, priv->videoAppsrc, "src_video");
        if (priv->audioGhostPad == NULL)
            priv->audioGhostPad = gst_tdk_src_expose_pad(self, priv->audioAppsrc, "src_audio");
    }

    GstStateChangeReturn result =
            GST_ELEMENT_CLASS(parent_class)->change_state(element, transition);

    if (transition == GST_STATE_CHANGE_READY_TO_PAUSED) {
        /* Our two pads are fixed for the lifetime of the bin. */
        gst_element_no_more_pads(GST_ELEMENT(self));
    }

    return result;
}

/* ---- GstURIHandler plumbing for the "tdk://" scheme -------------------- */

static const gchar* const* gst_tdk_src_get_protocols(GType)
{
    static const gchar* schemes[] = { "tdk", NULL };
    return schemes;
}

static GstURIType gst_tdk_src_uri_get_type(GType)
{
    return GST_URI_SRC;
}

static gchar* gst_tdk_src_get_uri(GstURIHandler* handler)
{
    GstTdkSrc* self = GST_TDK_SRC(handler);
    gchar* copy;

    GST_OBJECT_LOCK(self);
    copy = g_strdup(self->priv->location);
    GST_OBJECT_UNLOCK(self);
    return copy;
}

static gboolean gst_tdk_src_set_uri(GstURIHandler* handler, const gchar* uri, GError** error)
{
    GstTdkSrc* self = GST_TDK_SRC(handler);
    GstTdkSrcPrivate* priv = self->priv;

    if (GST_STATE(self) >= GST_STATE_PAUSED) {
        GST_ERROR_OBJECT(self, "Refusing URI update once the element has reached PAUSED");
        if (error != NULL)
            g_set_error(error, GST_URI_ERROR, GST_URI_ERROR_BAD_STATE,
                    "tdksrc location is immutable in PAUSED/PLAYING");
        return FALSE;
    }

    GST_OBJECT_LOCK(self);
    g_clear_pointer(&priv->location, g_free);
    if (uri != NULL)
        priv->location = g_strdup(uri);
    GST_OBJECT_UNLOCK(self);

    return TRUE;
}

static void gst_tdk_src_uri_handler_init(gpointer gIface, gpointer)
{
    GstURIHandlerInterface* iface = (GstURIHandlerInterface*) gIface;

    iface->get_protocols = gst_tdk_src_get_protocols;
    iface->get_type      = gst_tdk_src_uri_get_type;
    iface->get_uri       = gst_tdk_src_get_uri;
    iface->set_uri       = gst_tdk_src_set_uri;
}

/*
 * Each embedded appsrc raises its own EOS on the bus once its ES is drained.
 * We intercept and drop those child EOS notifications here so the bin does not
 * report EOS prematurely; the EOS events themselves keep travelling downstream
 * along the pads as usual. Everything else is forwarded to the base bin.
 */
static void gst_tdk_src_handle_message(GstBin* bin, GstMessage* message)
{
    if (GST_MESSAGE_TYPE(message) == GST_MESSAGE_EOS) {
        GST_DEBUG_OBJECT(GST_TDK_SRC(bin), "Dropping child EOS posted by %s",
                GST_MESSAGE_SRC_NAME(message));
        gst_message_unref(message);
        return;
    }

    GST_BIN_CLASS(parent_class)->handle_message(bin, message);
}

/*
 * Ghost-pad query handler ported from the production primesrc
 * (GStreamerMPBInstance::src_query_with_parent): forward every query to the
 * underlying appsrc target pad so scheduling/latency/caps queries resolve
 * correctly during preroll.
 */
static gboolean gst_tdk_src_ghost_query(GstPad* pad, GstObject* parent, GstQuery* query)
{
    GstPad* target = gst_ghost_pad_get_target(GST_GHOST_PAD_CAST(pad));
    gboolean result = FALSE;

    if (target != NULL) {
        result = gst_pad_query(target, query);
        gst_object_unref(target);
    }

    return result;
}

/* ---- Public accessors -------------------------------------------------- */

GstElement* gst_tdk_src_get_video_appsrc(GstTdkSrc* src)
{
    g_return_val_if_fail(GST_IS_TDK_SRC(src), NULL);
    return src->priv->videoAppsrc;
}

GstElement* gst_tdk_src_get_audio_appsrc(GstTdkSrc* src)
{
    g_return_val_if_fail(GST_IS_TDK_SRC(src), NULL);
    return src->priv->audioAppsrc;
}
