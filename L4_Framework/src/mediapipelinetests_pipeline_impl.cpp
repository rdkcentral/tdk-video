/**
 * @file mediapipelinetests_pipeline_impl.cpp
 * @brief Full implementation of pipeline management layer
 */

#include "mediapipelinetests_pipeline_management.h"
#include "mediapipelinetests_common_framework.h"
#include "ut_log_wrapper.h"
#include <gst/gst.h>
#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include <memory>
#include <sstream>

namespace {

void firstVideoFrame(GClosure* closure, GValue*, guint, const GValue*, gpointer, gpointer)
{
    auto* observation = static_cast<std::shared_ptr<FirstFrameObservation>*>(closure->data);
    (*observation)->received.store(true);
}

void releaseFirstFrameObservation(gpointer data, GClosure*)
{
    delete static_cast<std::shared_ptr<FirstFrameObservation>*>(data);
}

guint parsePlaybinFlags(GFlagsClass* flagsClass, const std::string& text, const char* key)
{
    if (text.empty() || text.back() == '+') {
        UT_LOG("ERROR: %s has an empty flag name: %s", key, text.c_str());
        throw std::runtime_error(std::string("Empty flag name in ") + key);
    }
    guint flags = 0;
    std::istringstream stream(text);
    std::string name;
    while (std::getline(stream, name, '+')) {
        const size_t start = name.find_first_not_of(" \t");
        if (start == std::string::npos) {
            UT_LOG("ERROR: %s has an empty flag name: %s", key, text.c_str());
            throw std::runtime_error(std::string("Empty flag name in ") + key);
        }
        name = name.substr(start, name.find_last_not_of(" \t") - start + 1);
        const GFlagsValue* value = g_flags_get_value_by_nick(flagsClass, name.c_str());
        if (!value) {
            UT_LOG("ERROR: Unsupported playbin flag in %s: %s", key, name.c_str());
            throw std::runtime_error("Unsupported playbin flag: " + name);
        }
        flags |= value->value;
    }
    return flags;
}

}

PipelineBusMonitor::PipelineBusMonitor(GstElement* pipeline, bool failOnError)
    : m_failOnError(failOnError)
{
    m_bus = gst_element_get_bus(pipeline);
    if (!m_bus) {
        UT_LOG("ERROR: Could not obtain pipeline bus");
        throw std::runtime_error("Could not obtain pipeline bus");
    }
    m_pipeline = GST_ELEMENT(gst_object_ref(pipeline));
    try {
        m_thread = std::thread(&PipelineBusMonitor::run, this);
    } catch (const std::system_error& error) {
        gst_object_unref(m_bus);
        gst_object_unref(m_pipeline);
        UT_LOG("ERROR: Could not start pipeline bus thread: %s", error.what());
        throw;
    }
    UT_LOG("[BUS] Started pipeline message-monitor thread");
}

PipelineBusMonitor::~PipelineBusMonitor()
{
    stop();
    gst_object_unref(m_bus);
    gst_object_unref(m_pipeline);
}

void PipelineBusMonitor::stop()
{
    m_stopping.store(true);
    if (m_thread.joinable()) {
        m_thread.join();
        UT_LOG("[BUS] Message-monitor thread joined");
    }
}

void PipelineBusMonitor::reportError()
{
    // Bus errors are asserted once by PhaseValidation::verifyNoBusErrors.
    if (hasError() && !m_errorReported.exchange(true)) {
        UT_LOG("ERROR: GstBus error latched; aborting verification");
    }
}

bool PipelineBusMonitor::setState(GstState state)
{
    std::lock_guard<std::mutex> lock(m_stateMutex);
    if (hasError() || m_stopping.load()) {
        return false;
    }
    return gst_element_set_state(m_pipeline, state) != GST_STATE_CHANGE_FAILURE;
}

void PipelineBusMonitor::run()
{
    for (;;) {
        const bool stopping = m_stopping.load();
        GstMessage* message = gst_bus_timed_pop_filtered(m_bus,
            stopping ? 0 : 100 * GST_MSECOND,
            static_cast<GstMessageType>(
                GST_MESSAGE_ERROR | GST_MESSAGE_EOS | GST_MESSAGE_STATE_CHANGED));
        if (!message) {
            if (stopping) {
                break;
            }
            continue;
        }
        const char* source = GST_MESSAGE_SRC(message) ?
            GST_OBJECT_NAME(GST_MESSAGE_SRC(message)) : "unknown";
        switch (GST_MESSAGE_TYPE(message)) {
            case GST_MESSAGE_ERROR: {
                GError* error = nullptr;
                gchar* debug = nullptr;
                gst_message_parse_error(message, &error, &debug);
                ++m_errorCount;
                UT_LOG("[BUS] ERROR from %s: %s", source, error ? error->message : "unknown");
                if (debug) {
                    UT_LOG("[BUS] Error details: %s", debug);
                }
                if (error) {
                    g_error_free(error);
                }
                g_free(debug);
                if (!m_failOnError) {
                    UT_LOG("[BUS] Log-only error policy: continuing without a bus-error assertion or teardown");
                    break;
                }
                m_error.store(true);
                UT_LOG("[BUS] Assertion failure pending; stopping pipeline immediately");
                {
                    std::lock_guard<std::mutex> lock(m_stateMutex);
                    if (gst_element_set_state(m_pipeline, GST_STATE_NULL) == GST_STATE_CHANGE_FAILURE) {
                        UT_LOG("ERROR: Could not stop pipeline after GstBus error");
                    }
                }
                break;
            }
            case GST_MESSAGE_EOS:
                m_eos.store(true);
                UT_LOG("[BUS] EOS from %s", source);
                break;
            case GST_MESSAGE_STATE_CHANGED: {
                if (GST_MESSAGE_SRC(message) != GST_OBJECT(m_pipeline)) {
                    break;
                }
                GstState oldState, newState, pendingState;
                gst_message_parse_state_changed(message, &oldState, &newState, &pendingState);
                UT_LOG("[BUS] %s state: %s -> %s (pending %s)", source,
                    gst_element_state_get_name(oldState), gst_element_state_get_name(newState),
                    gst_element_state_get_name(pendingState));
                break;
            }
            default:
                break;
        }
        gst_message_unref(message);
    }
}

PipelineHandle PipelineManagementLayer::createPipeline(
    const MediaPipelineConfiguration& config,
    const std::string& videoUrl,
    PipelineMediaType mediaType)
{
    const bool audioOnly = mediaType == PipelineMediaType::AudioOnly;
    UT_LOG("[PIPELINE] Creating %s pipeline with URL: %s",
        audioOnly ? "audio-only" : "audio/video", videoUrl.c_str());
    if (audioOnly && config.playbinAudioFlags.empty()) {
        UT_LOG("ERROR: Audio-only pipeline requires playbin_audio_flags");
        throw std::runtime_error("playbin_audio_flags is required for audio-only pipelines");
    }
    
    // Create playbin element
    GstElement* playbin = gst_element_factory_make("playbin", NULL);
    if (!playbin) {
        UT_LOG("ERROR: Could not create playbin element");
        throw std::runtime_error("Failed to create playbin element");
    }

    const auto unrefElement = [](GstElement* element) { gst_object_unref(element); };
    std::unique_ptr<GstElement, decltype(unrefElement)> creationGuard(playbin, unrefElement);
    const bool useVideoFlags = !audioOnly && !config.playbinVideoFlags.empty();
    if (useVideoFlags || !config.playbinAudioFlags.empty()) {
        GParamSpec* property = g_object_class_find_property(G_OBJECT_GET_CLASS(playbin), "flags");
        if (!property || !G_IS_PARAM_SPEC_FLAGS(property)) {
            UT_LOG("ERROR: Playbin does not expose a flags property");
            throw std::runtime_error("Playbin flags property is unavailable");
        }
        const auto unrefClass = [](GFlagsClass* flags) { g_type_class_unref(flags); };
        std::unique_ptr<GFlagsClass, decltype(unrefClass)> flagsClass(
            static_cast<GFlagsClass*>(g_type_class_ref(G_PARAM_SPEC_VALUE_TYPE(property))),
            unrefClass);
        if (!flagsClass) {
            UT_LOG("ERROR: Could not inspect playbin flags");
            throw std::runtime_error("Could not inspect playbin flags");
        }
        guint flags = 0;
        std::string applied;
        if (useVideoFlags) {
            flags |= parsePlaybinFlags(
                flagsClass.get(), config.playbinVideoFlags, "playbin_video_flags");
            applied = config.playbinVideoFlags;
        }
        if (!config.playbinAudioFlags.empty()) {
            flags |= parsePlaybinFlags(
                flagsClass.get(), config.playbinAudioFlags, "playbin_audio_flags");
            applied += (applied.empty() ? "" : "+") + config.playbinAudioFlags;
        }
        g_object_set(G_OBJECT(playbin), "flags", flags, NULL);
        UT_LOG("[PIPELINE] Playbin flags set: %s (mask=%u)", applied.c_str(), flags);
    } else {
        UT_LOG("[PIPELINE] Using GStreamer default playbin flags");
    }

    // Set video URI
    g_object_set(G_OBJECT(playbin), "uri", videoUrl.c_str(), NULL);
    
    // Create video sink
    // Audio-only pipelines attach only the audio sink.
    GstElement* videoSink = NULL;
    if (!audioOnly && !config.videoSink.empty()) {
        videoSink = gst_element_factory_make(config.videoSink.c_str(), NULL);
        if (videoSink) {
            g_object_set(G_OBJECT(playbin), "video-sink", videoSink, NULL);
            UT_LOG("[PIPELINE] Video sink created: %s", config.videoSink.c_str());
        } else {
            UT_LOG("WARNING: Could not create video sink: %s", config.videoSink.c_str());
        }
    }
    
    // Create audio sink
    GstElement* audioSink = NULL;
    if (!config.audioSink.empty()) {
        audioSink = gst_element_factory_make(config.audioSink.c_str(), NULL);
        if (audioSink) {
            g_object_set(G_OBJECT(playbin), "audio-sink", audioSink, NULL);
            UT_LOG("[PIPELINE] Audio sink created: %s", config.audioSink.c_str());
        } else {
            UT_LOG("WARNING: Could not create audio sink: %s", config.audioSink.c_str());
        }
    }
    
    // Create pipeline handle
    PipelineHandle handle;
    handle.playbin = playbin;
    handle.videoSink = videoSink;
    handle.audioSink = audioSink;
    handle.checkPts = config.checkPts;
    handle.videoFramesValidationEnabled = config.videoFramesValidationEnabled;
    handle.audioFramesValidationEnabled = config.audioFramesValidationEnabled;
    handle.ptsErrorTolerance = config.ptsErrorTolerance;
    handle.videoFramesTolerance = config.videoFramesTolerance;
    handle.audioFramesTolerance = config.audioFramesTolerance;
    handle.positionToleranceMs = config.positionToleranceMs;
    handle.pausePositionToleranceMs = config.pausePositionToleranceMs;
    handle.sinkStallTimeoutMs = config.sinkStallTimeoutMs;
    handle.verificationLogIntervalMs = config.verificationLogIntervalMs;
    handle.verboseVerification = config.logLevel == "debug";

    guint activeFlags = 0;
    g_object_get(playbin, "flags", &activeFlags, nullptr);
    GParamSpec* flagProperty = g_object_class_find_property(G_OBJECT_GET_CLASS(playbin), "flags");
    if (!flagProperty || !G_IS_PARAM_SPEC_FLAGS(flagProperty)) {
        UT_LOG("ERROR: Playbin flags property is unavailable");
        throw std::runtime_error("Playbin flags property is unavailable");
    }
    const auto releaseFlags = [](GFlagsClass* flags) { g_type_class_unref(flags); };
    std::unique_ptr<GFlagsClass, decltype(releaseFlags)> activeClass(
        static_cast<GFlagsClass*>(g_type_class_ref(G_PARAM_SPEC_VALUE_TYPE(flagProperty))),
        releaseFlags);
    if (!activeClass) {
        UT_LOG("ERROR: Could not inspect active playbin flags");
        throw std::runtime_error("Could not inspect active playbin flags");
    }
    const GFlagsValue* videoFlag = g_flags_get_value_by_nick(activeClass.get(), "video");
    const GFlagsValue* audioFlag = g_flags_get_value_by_nick(activeClass.get(), "audio");
    handle.videoEnabled = videoFlag && (activeFlags & videoFlag->value) != 0;
    handle.audioEnabled = audioFlag && (activeFlags & audioFlag->value) != 0;
    handle.audioOnly = audioOnly;
    if (audioOnly) {
        if (!handle.audioEnabled || handle.videoEnabled) {
            UT_LOG("ERROR: playbin_audio_flags must enable audio and not video for audio-only pipelines");
            throw std::runtime_error("Invalid playbin_audio_flags for audio-only pipeline");
        }
        UT_LOG("[SKIP] Audio-only pipeline: first-frame, video PTS and video frame checks skipped");
    }

    if (videoSink && handle.videoEnabled &&
        g_signal_lookup("first-video-frame-callback", G_OBJECT_TYPE(videoSink)) != 0) {
        handle.firstFrame = std::make_shared<FirstFrameObservation>();
        GClosure* closure = g_closure_new_simple(sizeof(GClosure),
            new std::shared_ptr<FirstFrameObservation>(handle.firstFrame));
        g_closure_set_marshal(closure, firstVideoFrame);
        g_closure_add_finalize_notifier(
            closure, closure->data, releaseFirstFrameObservation);
        handle.firstFrameHandler = g_signal_connect_closure(
            videoSink, "first-video-frame-callback", closure, FALSE);
        if (!handle.firstFrameHandler) {
            UT_LOG("ERROR: Could not register first video frame callback");
            throw std::runtime_error("Could not register first video frame callback");
        }
        UT_LOG("[PIPELINE] Registered first-video-frame-callback before PLAYING");
    } else if (!audioOnly) {
        UT_LOG("[SKIP] First video frame signal unavailable or video disabled");
    }
    handle.busMonitor = std::make_shared<PipelineBusMonitor>(playbin, config.failOnBusError);
    creationGuard.release();
    
    UT_LOG("[PIPELINE] Pipeline created successfully");
    return handle;
}

void PipelineManagementLayer::destroyPipeline(PipelineHandle& pipeline)
{
    UT_LOG("[PIPELINE] Destroying pipeline...");
    
    if (pipeline.playbin) {
        if (pipeline.busMonitor) {
            pipeline.busMonitor->stop();
            pipeline.busMonitor->reportError();
        }
        // Set to NULL state
        gst_element_set_state(pipeline.playbin, GST_STATE_NULL);
        if (pipeline.firstFrameHandler && pipeline.videoSink) {
            g_signal_handler_disconnect(pipeline.videoSink, pipeline.firstFrameHandler);
        }
        
        // Unref the element
        gst_object_unref(GST_OBJECT(pipeline.playbin));
        pipeline = {};
        
        UT_LOG("[PIPELINE] Pipeline destroyed");
    }
}

bool PipelineManagementLayer::setState_Playing(const PipelineHandle& pipeline)
{
    if (pipeline.busMonitor && pipeline.busMonitor->hasError()) {
        pipeline.busMonitor->reportError();
        UT_LOG("ERROR: Cannot set PLAYING after a pipeline bus error");
        return false;
    }
    if (!pipeline.playbin) {
        UT_LOG("ERROR: Pipeline playbin is null");
        return false;
    }
    
    UT_LOG("[STATE] Setting pipeline to PLAYING...");
    const bool changed = pipeline.busMonitor ?
        pipeline.busMonitor->setState(GST_STATE_PLAYING) :
        gst_element_set_state(pipeline.playbin, GST_STATE_PLAYING) != GST_STATE_CHANGE_FAILURE;

    if (!changed) {
        if (pipeline.busMonitor) {
            pipeline.busMonitor->reportError();
        }
        UT_LOG("ERROR: Could not set state to PLAYING");
        return false;
    }
    
    UT_LOG("[STATE] Pipeline set to PLAYING");
    return true;
}

bool PipelineManagementLayer::setState_Paused(const PipelineHandle& pipeline)
{
    if (pipeline.busMonitor && pipeline.busMonitor->hasError()) {
        pipeline.busMonitor->reportError();
        UT_LOG("ERROR: Cannot set PAUSED after a pipeline bus error");
        return false;
    }
    if (!pipeline.playbin) {
        UT_LOG("ERROR: Pipeline playbin is null");
        return false;
    }
    
    UT_LOG("[STATE] Setting pipeline to PAUSED...");
    const bool changed = pipeline.busMonitor ?
        pipeline.busMonitor->setState(GST_STATE_PAUSED) :
        gst_element_set_state(pipeline.playbin, GST_STATE_PAUSED) != GST_STATE_CHANGE_FAILURE;

    if (!changed) {
        if (pipeline.busMonitor) {
            pipeline.busMonitor->reportError();
        }
        UT_LOG("ERROR: Could not set state to PAUSED");
        return false;
    }
    
    UT_LOG("[STATE] Pipeline set to PAUSED");
    return true;
}

bool PipelineManagementLayer::getPosition(
    const PipelineHandle& pipeline,
    gint64& position)
{
    if (pipeline.busMonitor && pipeline.busMonitor->hasError()) {
        pipeline.busMonitor->reportError();
        UT_LOG("ERROR: Cannot query position after a pipeline bus error");
        return false;
    }
    if (!pipeline.playbin) {
        return false;
    }
    
    gint64 pos = 0;
    GstFormat fmt = GST_FORMAT_TIME;
    
    if (gst_element_query_position(pipeline.playbin, fmt, &pos) && pos >= 0) {
        position = pos;
        return true;
    }
    
    UT_LOG("ERROR: Failed to query a valid pipeline position");
    return false;
}

bool PipelineManagementLayer::seek(
    const PipelineHandle& pipeline,
    gint64 position)
{
    if (pipeline.busMonitor && pipeline.busMonitor->hasError()) {
        pipeline.busMonitor->reportError();
        UT_LOG("ERROR: Cannot seek after a pipeline bus error");
        return false;
    }
    if (!pipeline.playbin) {
        UT_LOG("ERROR: Pipeline playbin is null");
        return false;
    }
    
    UT_LOG("[SEEK] Seeking to %" G_GINT64_FORMAT " ns", position);
    
    GstEvent* seek_event = gst_event_new_seek(
        1.0, 
        GST_FORMAT_TIME,
        GST_SEEK_FLAG_FLUSH,
        GST_SEEK_TYPE_SET,
        position,
        GST_SEEK_TYPE_NONE,
        -1);
    
    if (gst_element_send_event(pipeline.playbin, seek_event)) {
        UT_LOG("[SEEK] Seek successful");
        return true;
    }
    
    UT_LOG("[SEEK] Seek failed");
    return false;
}

bool PipelineManagementLayer::setPlaybackRate(
    const PipelineHandle& pipeline,
    double rate)
{
    if (!pipeline.playbin) {
        UT_LOG("ERROR: Pipeline playbin is null");
        return false;
    }
    
    UT_LOG("[RATE] Setting playback rate to %f", rate);
    
    gint64 pos = 0;
    gst_element_query_position(pipeline.playbin, GST_FORMAT_TIME, &pos);
    
    GstEvent* seek_event = gst_event_new_seek(
        rate,
        GST_FORMAT_TIME,
        GST_SEEK_FLAG_FLUSH,
        GST_SEEK_TYPE_SET,
        pos,
        GST_SEEK_TYPE_NONE,
        -1);
    
    if (gst_element_send_event(pipeline.playbin, seek_event)) {
        UT_LOG("[RATE] Rate change successful");
        return true;
    }
    
    UT_LOG("[RATE] Rate change failed");
    return false;
}
