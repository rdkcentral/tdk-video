/**
 * @file mediapipelinetests_pipeline_management.h
 * @brief Pipeline creation and management interface
 * 
 * Abstracts GStreamer pipeline construction from test logic.
 * Platform-specific implementations create appropriate pipelines.
 */

#ifndef MP_PIPELINE_MANAGEMENT_H
#define MP_PIPELINE_MANAGEMENT_H

#include "mediapipelinetests_configuration.h"
#include <gst/gst.h>
#include <string>
#include <atomic>
#include <memory>
#include <thread>
#include <mutex>

class PipelineBusMonitor
{
public:
    explicit PipelineBusMonitor(GstElement* pipeline, bool failOnError = true);
    ~PipelineBusMonitor();
    PipelineBusMonitor(const PipelineBusMonitor&) = delete;
    PipelineBusMonitor& operator=(const PipelineBusMonitor&) = delete;

    void stop();
    bool hasError() const { return m_error.load(); }
    bool hasEos() const { return m_eos.load(); }
    unsigned int errorCount() const { return m_errorCount.load(); }
    bool failOnError() const { return m_failOnError; }
    void reportError();
    bool setState(GstState state);

private:
    void run();
    GstElement* m_pipeline = nullptr;
    GstBus* m_bus = nullptr;
    std::thread m_thread;
    std::atomic<bool> m_stopping{false};
    std::atomic<bool> m_error{false};
    std::atomic<bool> m_eos{false};
    std::atomic<bool> m_errorReported{false};
    std::atomic<unsigned int> m_errorCount{0};
    std::mutex m_stateMutex;
    bool m_failOnError;
};

struct FirstFrameObservation
{
    std::atomic<bool> received{false};
};

enum class PipelineMediaType
{
    AudioVideo,
    AudioOnly
};

/**
 * @struct PipelineHandle
 * @brief Opaque handle to a managed pipeline
 * 
 * Encapsulates pipeline element and associated metadata.
 * Tests use this handle without knowing internal GStreamer details.
 */
struct PipelineHandle
{
    // TODO: Define pipeline handle contents
    // - GstElement* playbin;
    // - GstElement* audioSink;
    // - GstElement* videoSink;
    // - Additional metadata for validation
    GstElement* playbin = nullptr;
    GstElement* audioSink = nullptr;
    GstElement* videoSink = nullptr;
    std::shared_ptr<FirstFrameObservation> firstFrame;
    std::shared_ptr<PipelineBusMonitor> busMonitor;
    gulong firstFrameHandler = 0;
    bool videoEnabled = true;
    bool audioEnabled = true;
    bool audioOnly = false;
    bool checkPts = true;
    bool videoFramesValidationEnabled = true;
    bool audioFramesValidationEnabled = true;
    unsigned int ptsErrorTolerance = 1;
    unsigned int videoFramesTolerance = 1;
    unsigned int audioFramesTolerance = 1;
    unsigned int positionToleranceMs = 900;
    unsigned int pausePositionToleranceMs = 100;
    unsigned int sinkStallTimeoutMs = 2000;
    unsigned int verificationLogIntervalMs = 1000;
    bool verboseVerification = false;
};

/**
 * @class PipelineManagementLayer
 * @brief Platform-independent pipeline creation and management
 * 
 * Creates pipelines configured from platform.ini.
 * Delegates platform-specific decisions to implementations.
 */
class PipelineManagementLayer
{
public:
    /**
     * @brief Create a pipeline for testing
     * @param config Configuration loaded from platform.ini
     * @param videoUrl URL or path to video file
     * @return Initialized and ready-to-use PipelineHandle
     * 
     * TODO: Implement pipeline creation
     * Steps:
     *   1. Validate configuration
     *   2. Create playbin element
     *   3. Set video URL on playbin
     *   4. Get or create audio sink from config
     *   5. Get or create video sink from config
     *   6. Configure sink properties (filters, volume, etc.)
     *   7. Set up bus for message handling
     *   8. Return PipelineHandle with all elements
     * 
     * Platform-specific:
     *   - Element names come from config
     *   - Audio/video sink creation differs by platform
     *   - Filter configuration varies by platform
     */
    static PipelineHandle createPipeline(
        const MediaPipelineConfiguration& config,
        const std::string& videoUrl,
        PipelineMediaType mediaType = PipelineMediaType::AudioVideo);
    
    /**
     * @brief Destroy pipeline and clean up resources
     * @param pipeline Handle to pipeline to destroy
     * 
     * TODO: Implement pipeline cleanup
     * Steps:
     *   1. Set pipeline state to NULL
     *   2. Unref all elements
     *   3. Unref bus
     *   4. Clear handle
     * 
     * Note: Must handle partial initialization gracefully
     */
    static void destroyPipeline(PipelineHandle& pipeline);
    
    /**
     * @brief Set pipeline to PLAYING state
     * @param pipeline Pipeline handle
     * @return true if state change successful, false otherwise
     * 
     * TODO: Implement state change to PLAYING
     * Steps:
     *   1. Get current state
     *   2. If already PLAYING, return true
     *   3. Set state to PLAYING
     *   4. Wait for state change to complete
     *   5. Return success status
     * 
     * Note: May be asynchronous on some platforms
     */
    static bool setState_Playing(const PipelineHandle& pipeline);
    
    /**
     * @brief Set pipeline to PAUSED state
     * @param pipeline Pipeline handle
     * @return true if state change successful, false otherwise
     * 
     * TODO: Implement state change to PAUSED
     * Steps:
     *   1. Get current state
     *   2. If already PAUSED, return true
     *   3. Set state to PAUSED
     *   4. Wait for state change to complete
     *   5. Return success status
     */
    static bool setState_Paused(const PipelineHandle& pipeline);
    
    /**
     * @brief Get current playback position
     * @param pipeline Pipeline handle
     * @param position [out] Current position in nanoseconds
     * @return true if query successful, false otherwise
     * 
     * TODO: Implement position query
     * Steps:
     *   1. Query pipeline for current position
     *   2. Convert to nanoseconds if needed
     *   3. Store in position reference
     *   4. Return success status
     */
    static bool getPosition(const PipelineHandle& pipeline, gint64& position);
    
    /**
     * @brief Seek to specified position
     * @param pipeline Pipeline handle
     * @param position Target position in nanoseconds
     * @return true if seek initiated successfully, false otherwise
     * 
     * TODO: Implement seek operation
     * Steps:
     *   1. Validate position is within stream duration
     *   2. Issue seek event with GST_SEEK_FLAG_FLUSH
     *   3. Wait for seek to complete
     *   4. Return success status
     */
    static bool seek(const PipelineHandle& pipeline, gint64 position);
    
    /**
     * @brief Set playback rate (for trick play operations)
     * @param pipeline Pipeline handle
     * @param rate Desired rate (1.0 = normal, 2.0 = 2x speed, -1.0 = reverse, etc.)
     * @return true if rate change successful, false otherwise
     * 
     * TODO: Implement rate setting
     * Steps:
     *   1. Validate rate is non-zero
     *   2. Get current position
     *   3. Issue seek with desired rate
     *   4. Wait for completion
     *   5. Return success status
     */
    static bool setPlaybackRate(const PipelineHandle& pipeline, double rate);
    
};

#endif // MP_PIPELINE_MANAGEMENT_H
