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
#ifndef GstTdkSrc_h
#define GstTdkSrc_h

#include <gst/gst.h>

G_BEGIN_DECLS

#define GST_TDK_TYPE_SRC            (gst_tdk_src_get_type ())
#define GST_TDK_SRC(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TDK_TYPE_SRC, GstTdkSrc))
#define GST_TDK_SRC_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GST_TDK_TYPE_SRC, GstTdkSrcClass))
#define GST_IS_TDK_SRC(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TDK_TYPE_SRC))
#define GST_IS_TDK_SRC_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TDK_TYPE_SRC))

typedef struct _GstTdkSrc        GstTdkSrc;
typedef struct _GstTdkSrcClass   GstTdkSrcClass;
typedef struct _GstTdkSrcPrivate GstTdkSrcPrivate;

struct _GstTdkSrc {
    GstBin parent;
    GstTdkSrcPrivate *priv;
};

struct _GstTdkSrcClass {
    GstBinClass parentClass;
};

GType gst_tdk_src_get_type(void);

/*
 * Hand back the internal appsrc pushers so the host application can inject
 * video / audio elementary-stream buffers straight into tdksrc. These elements
 * belong to the bin, so callers must not take or drop a reference on them.
 */
GstElement* gst_tdk_src_get_video_appsrc(GstTdkSrc* src);
GstElement* gst_tdk_src_get_audio_appsrc(GstTdkSrc* src);

G_END_DECLS

#endif
