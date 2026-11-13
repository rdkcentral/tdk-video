/**
 * @file mediapipelinetests_verification.h
 * @brief Test verification interfaces and utilities
 * 
 * Separates verification logic from test execution logic.
 * Each verification scenario has dedicated interface.
 */

#ifndef MP_VERIFICATION_H
#define MP_VERIFICATION_H

#include "mediapipelinetests_pipeline_management.h"
#include <gst/gst.h>
#include <string>

/**
 * @enum VerificationResult
 * @brief Result codes for verification operations
 * 
 * Used to report verification success/failure to test framework.
 * Each verification counts as ONE assertion for test statistics.
 */
enum class VerificationResult
{
    SUCCESS,           ///< Verification passed
    FAILURE,           ///< Verification failed
    TIMEOUT,           ///< Verification timed out
    NOT_APPLICABLE,    ///< Verification not applicable to this scenario
    ERROR              ///< Internal error during verification
};

/**
 * @enum CheckOutcome
 * @brief Outcome of one granular check; only PASSED/FAILED become assertions
 */
enum class CheckOutcome
{
    NOT_RUN,   ///< Not evaluated (test aborted earlier or bus error)
    PASSED,
    FAILED,
    SKIPPED    ///< Not applicable/supported (audio-only, disabled, missing property)
};

/**
 * @struct PhaseCheckResults
 * @brief Per-check outcomes for one PLAYING or PAUSED verification phase
 */
struct PhaseCheckResults
{
    CheckOutcome state = CheckOutcome::NOT_RUN;
    CheckOutcome position = CheckOutcome::NOT_RUN;
    CheckOutcome pts = CheckOutcome::NOT_RUN;
    CheckOutcome videoFrames = CheckOutcome::NOT_RUN;
    CheckOutcome audioFrames = CheckOutcome::NOT_RUN;
};

/**
 * @class PhaseValidation
 * @brief Granular checks used to report one assertion per verified property
 */
class PhaseValidation
{
public:
    /** Wait for the pipeline to reach a state */
    static CheckOutcome verifyState(
        const PipelineHandle& pipeline, GstState expected, unsigned int timeoutSeconds);

    /** Wait for the video sink first-frame signal; SKIPPED when unavailable */
    static CheckOutcome verifyFirstFrame(
        const PipelineHandle& pipeline, unsigned int timeoutSeconds);

    /**
     * Poll position, PTS and rendered counters every 100 ms. PLAYING requires
     * advancement; PAUSED requires them frozen. Stops at the first FAILED check.
     * results.state is set to FAILED if the state changes during monitoring.
     */
    static VerificationResult monitorPhase(
        const PipelineHandle& pipeline, unsigned int durationMs, bool paused,
        bool stopWhenAdvanced, PhaseCheckResults& results);

    /** Stop the bus monitor (draining it), set NULL and verify the NULL state */
    static CheckOutcome verifyNullState(const PipelineHandle& pipeline);

    /** Verify no GstBus ERROR messages were received during the pipeline lifetime */
    static CheckOutcome verifyNoBusErrors(const PipelineHandle& pipeline);
};

/**
 * @class PlaybackValidation
 * @brief Validate basic playback scenarios
 * 
 * Handles verification of:
 * - Playback start (does media begin playing)
 * - Playback duration (does stream play for expected time)
 * - EOS (End Of Stream) detection
 * - Frame rendering on video sink
 */
class PlaybackValidation
{
public:
    /**
     * @brief Verify that playback starts successfully
     * @param pipeline Pipeline to test
     * @param timeoutSeconds Timeout for verification
     * @return VerificationResult (counts as 1 assertion)
     * 
     * Waits for PLAYING and the first-video-frame callback when supported,
     * then confirms position and supported sink metrics advance.
     * Unsupported sink signals/properties are explicitly logged as skipped.
     * 
     * Note: Counts as ONE assertion regardless of how many
     *       signals checked internally
     */
    static VerificationResult verifyPlaybackStart(
        const PipelineHandle& pipeline,
        unsigned int timeoutSeconds);
    
    /**
     * @brief Verify playback continues for specified duration
     * @param pipeline Pipeline to test
     * @param durationSeconds Expected minimum playback duration
     * @return VerificationResult (counts as 1 assertion)
     * 
     * Polls position, video PTS and video/audio rendered counters every 100 ms.
     * Compares position advancement to monotonic elapsed time within the
     * configured tolerance, rejecting stalled or backward sink metrics.
     * 
     * Internal loop verification:
     *   - May monitor position multiple times during wait
     *   - Counts as SINGLE assertion
     *   - Reports aggregate result (passed or failed)
     */
    static VerificationResult verifyPlaybackDuration(
        const PipelineHandle& pipeline,
        unsigned int durationSeconds);
    
    /**
     * @brief Verify EOS (End Of Stream) is received
     * @param pipeline Pipeline to test
     * @param timeoutSeconds Timeout for EOS
     * @return VerificationResult (counts as 1 assertion)
     * 
     * TODO: Implement EOS detection
     * Steps:
     *   1. Monitor bus for GST_MESSAGE_EOS
     *   2. Wait up to timeoutSeconds
     *   3. Return SUCCESS if EOS received, FAILURE if timeout
     */
    static VerificationResult verifyEndOfStream(
        const PipelineHandle& pipeline,
        unsigned int timeoutSeconds);
};

/**
 * @class PauseResumeValidation
 * @brief Validate pause and resume operations
 * 
 * Handles verification of:
 * - Pipeline transitions to PAUSED state
 * - Playback actually pauses (position stops advancing)
 * - Resume from pause works correctly
 */
class PauseResumeValidation
{
public:
    /**
     * @brief Verify pipeline is in PAUSED state
     * @param pipeline Pipeline to test
     * @param timeoutSeconds Timeout for state change
     * @return VerificationResult (counts as 1 assertion)
     * 
     * TODO: Implement pause state verification
     * Steps:
     *   1. Get current pipeline state
     *   2. Verify it equals GST_STATE_PAUSED
     *   3. Return SUCCESS if PAUSED, FAILURE if not
     */
    static VerificationResult verifyPausedState(
        const PipelineHandle& pipeline,
        unsigned int timeoutSeconds);
    
    /**
     * @brief Verify position doesn't advance while paused
     * @param pipeline Pipeline to test
     * @param durationSeconds Time to monitor for advancement
     * @return VerificationResult (counts as 1 assertion)
     * 
     * Polls position and supported sink metrics every 100 ms.
     * Position must remain within the pause tolerance; video PTS and
     * video/audio rendered counters must remain equal to their baselines.
     * 
     * Polling verification:
     *   - May poll position multiple times during wait
     *   - Counts as SINGLE assertion
     *   - Reports aggregate result
     */
    static VerificationResult verifyPlaybackPaused(
        const PipelineHandle& pipeline,
        unsigned int durationSeconds);
    
    /**
     * @brief Verify playback resumes after pause
     * @param pipeline Pipeline to test
     * @param timeoutSeconds Timeout for resume
     * @return VerificationResult (counts as 1 assertion)
     * 
     * Caller requests PLAYING before invoking this method.
     * Waits for that state and confirms position and supported sink metrics
     * advance within one shared monotonic timeout budget.
     */
    static VerificationResult verifyPlaybackResumes(
        const PipelineHandle& pipeline,
        unsigned int timeoutSeconds);
};

/**
 * @class FrameValidation
 * @brief Validate video frame rendering
 * 
 * Handles verification of:
 * - Frames being rendered on video sink
 * - Frame rate consistency
 * - Frame drops within acceptable threshold
 * - Resolution changes
 */
class FrameValidation
{
public:
    /**
     * @brief Verify frames are being rendered
     * @param pipeline Pipeline to test
     * @param minFramesExpected Minimum frames expected
     * @param timeoutSeconds Timeout for frame collection
     * @return VerificationResult (counts as 1 assertion)
     * 
     * TODO: Implement frame rendering verification
     * Steps:
     *   1. Hook into video sink callbacks
     *   2. Count frames over timeoutSeconds
     *   3. Verify count >= minFramesExpected
     *   4. Return SUCCESS if enough frames, FAILURE if not
     * 
     * Loop verification:
     *   - Callback invoked many times per second
     *   - Each frame checked internally
     *   - Counts as SINGLE assertion
     *   - Reports overall result (passed or failed)
     */
    static VerificationResult verifyFramesRendered(
        const PipelineHandle& pipeline,
        unsigned int minFramesExpected,
        unsigned int timeoutSeconds);
    
    /**
     * @brief Verify frame drops are within acceptable limit
     * @param pipeline Pipeline to test
     * @param maxAllowedDrops Maximum acceptable frame drops
     * @param monitorDurationSeconds Duration to monitor for drops
     * @return VerificationResult (counts as 1 assertion)
     * 
     * TODO: Implement frame drop verification
     * Steps:
     *   1. Monitor video sink statistics
     *   2. Track dropped vs. rendered frames
     *   3. Calculate drop percentage
     *   4. Verify drops <= maxAllowedDrops
     *   5. Return SUCCESS if within limit, FAILURE otherwise
     * 
     * Continuous monitoring:
     *   - May check drop count repeatedly during monitoring
     *   - Counts as SINGLE assertion
     *   - Reports aggregate frame drop count
     */
    static VerificationResult verifyFrameDropsAcceptable(
        const PipelineHandle& pipeline,
        unsigned int maxAllowedDrops,
        unsigned int monitorDurationSeconds);
};

/**
 * @class AudioValidation
 * @brief Validate audio playback
 * 
 * Handles verification of:
 * - Audio sink receiving audio data
 * - Audio underflow detection
 * - PTS (Presentation Time Stamp) continuity
 * - Sampling rate and bit depth
 */
class AudioValidation
{
public:
    /**
     * @brief Verify audio sink is receiving samples
     * @param pipeline Pipeline to test
     * @param minSamplesExpected Minimum audio samples
     * @param timeoutSeconds Timeout for sample collection
     * @return VerificationResult (counts as 1 assertion)
     * 
     * TODO: Implement audio sample verification
     * Steps:
     *   1. Hook into audio sink callbacks
     *   2. Count samples over timeoutSeconds
     *   3. Verify count >= minSamplesExpected
     *   4. Return SUCCESS if samples present, FAILURE if not
     */
    static VerificationResult verifyAudioSamples(
        const PipelineHandle& pipeline,
        unsigned int minSamplesExpected,
        unsigned int timeoutSeconds);
    
    /**
     * @brief Verify audio underflow signals
     * @param pipeline Pipeline to test
     * @param timeoutSeconds Timeout for underflow detection
     * @return VerificationResult (counts as 1 assertion)
     * 
     * TODO: Implement audio underflow detection
     * Steps:
     *   1. Monitor bus for GST_MESSAGE_BUFFERING
     *   2. Watch for audio buffer starvation
     *   3. Return SUCCESS if no underflow, FAILURE if underflow detected
     */
    static VerificationResult verifyNoAudioUnderflow(
        const PipelineHandle& pipeline,
        unsigned int timeoutSeconds);
};

/**
 * @class SyncValidation
 * @brief Validate audio/video synchronization
 * 
 * Handles verification of:
 * - AV sync within tolerance
 * - PTS gap detection
 * - Timestamp continuity
 */
class SyncValidation
{
public:
    /**
     * @brief Verify audio and video are synchronized
     * @param pipeline Pipeline to test
     * @param syncToleranceMs Maximum acceptable sync drift
     * @param monitorDurationSeconds Duration to monitor sync
     * @return VerificationResult (counts as 1 assertion)
     * 
     * TODO: Implement AV sync verification
     * Steps:
     *   1. Collect audio and video timestamps
     *   2. Calculate sync difference for each frame
     *   3. Verify all differences <= syncToleranceMs
     *   4. Return SUCCESS if in sync, FAILURE if drift detected
     * 
     * Repeated measurements:
     *   - May measure sync 30+ times during monitoring
     *   - Each measurement checked internally
     *   - Counts as SINGLE assertion
     *   - Reports aggregate sync status
     */
    static VerificationResult verifyAVSync(
        const PipelineHandle& pipeline,
        unsigned int syncToleranceMs,
        unsigned int monitorDurationSeconds);
    
    /**
     * @brief Verify PTS (Presentation Time Stamp) continuity
     * @param pipeline Pipeline to test
     * @param monitorDurationSeconds Duration to monitor PTS
     * @param maxJitterMs Maximum acceptable PTS jitter
     * @return VerificationResult (counts as 1 assertion)
     * 
     * Checks PTS values at regular intervals for continuity and jitter.
     * Each measurement checked internally, counts as SINGLE assertion.
     */
    static VerificationResult verifyPTSContinuity(
        const PipelineHandle& pipeline,
        unsigned int monitorDurationSeconds,
        gint64 maxJitterMs);
    
    /**
     * @brief Verify playback position accuracy
     * @param pipeline Pipeline to test
     * @param monitorDurationSeconds Duration to monitor
     * @param maxPositionDriftMs Maximum acceptable drift from expected
     * @return VerificationResult (counts as 1 assertion)
     * 
     * Compares actual position vs expected position over time.
     * Typical tolerances: 50ms for normal playback, 100ms for network streams.
     */
    static VerificationResult verifyPositionAccuracy(
        const PipelineHandle& pipeline,
        unsigned int monitorDurationSeconds,
        gint64 maxPositionDriftMs);
};

/**
 * @class WesterosValidation
 * @brief Validate Broadcom Westeros-specific features
 * 
 * Handles Broadcom platform-specific verification:
 * - First frame signal from westerossink
 * - Buffer underflow detection
 * - Video resolution and capabilities
 * - Secure Video Path (SVP) support
 * - Graphics mode and refresh rate
 */
class WesterosValidation
{
public:
    /**
     * @brief Verify first frame signal received from westerossink
     * @param pipeline Pipeline to test
     * @param timeoutSeconds Timeout for first frame signal
     * @return VerificationResult (counts as 1 assertion)
     * 
     * Monitors for first frame buffer signal which indicates:
     * - Westerossink is initialized
     * - Video decoding started
     * - Frames are being prepared for display
     */
    static VerificationResult verifyFirstFrameSignal(
        const PipelineHandle& pipeline,
        unsigned int timeoutSeconds);
    
    /**
     * @brief Verify first audio frame signal received
     * @param pipeline Pipeline to test
     * @param timeoutSeconds Timeout for first audio frame
     * @return VerificationResult (counts as 1 assertion)
     * 
     * Monitors for first audio frame signal (from brcmaudiodecoder).
     * Indicates audio decoding and pipeline sync point.
     */
    static VerificationResult verifyFirstAudioFrameSignal(
        const PipelineHandle& pipeline,
        unsigned int timeoutSeconds);
    
    /**
     * @brief Verify Westerossink buffer underflow detection
     * @param pipeline Pipeline to test
     * @param monitorDurationSeconds Duration to monitor for underflow
     * @return VerificationResult (counts as 1 assertion)
     * 
     * Monitors bus for underflow signals. SUCCESS if NO underflow detected
     * during monitoring period. FAILURE if underflow occurs.
     * 
     * Underflow signals come from westerossink when display buffer starves.
     */
    static VerificationResult verifyNoWesterosUnderflow(
        const PipelineHandle& pipeline,
        unsigned int monitorDurationSeconds);
    
    /**
     * @brief Verify audio underflow signals and FIFO status
     * @param pipeline Pipeline to test
     * @param monitorDurationSeconds Duration to monitor
     * @return VerificationResult (counts as 1 assertion)
     * 
     * Monitors for audio underflow from brcmaudiodecoder.
     * Checks FIFO depth and frame counts in underflow messages.
     */
    static VerificationResult verifyNoAudioUnderflowSignal(
        const PipelineHandle& pipeline,
        unsigned int monitorDurationSeconds);
    
    /**
     * @brief Verify video resolution capabilities
     * @param pipeline Pipeline to test
     * @param expectedWidth Expected video width
     * @param expectedHeight Expected video height
     * @return VerificationResult (counts as 1 assertion)
     * 
     * Verifies westerossink reports correct resolution.
     * Also checks 4K capability status from sink properties.
     */
    static VerificationResult verifyVideoResolution(
        const PipelineHandle& pipeline,
        unsigned int expectedWidth,
        unsigned int expectedHeight);
    
    /**
     * @brief Verify Secure Video Path (SVP) support
     * @param pipeline Pipeline to test
     * @return VerificationResult (counts as 1 assertion)
     * 
     * Verifies that westerossink has SVP enabled for protected content.
     * Checks both video and audio decoder SVP capability flags.
     */
    static VerificationResult verifySVPSupport(
        const PipelineHandle& pipeline);
    
    /**
     * @brief Verify Westeros graphics mode and refresh rate
     * @param pipeline Pipeline to test
     * @return VerificationResult (counts as 1 assertion)
     * 
     * Verifies WESTEROS_GL_MODE and refresh rate settings.
     * Also checks graphics surface initialization success.
     */
    static VerificationResult verifyWesterosGraphicsMode(
        const PipelineHandle& pipeline);
};

#endif // MP_VERIFICATION_H
