/**
 * @file mediapipelinetests_test_suite.cpp
 * @brief Test suite implementation using ut-core framework
 * 
 * Contains individual test cases that exercise MediaPipelineTests functionality.
 * Each test is a ut-core test function registered with the framework.
 */

#include "mediapipelinetests_common_framework.h"
#include "ut_log_wrapper.h"
#include "mediapipelinetests_pipeline_management.h"
#include "mediapipelinetests_verification.h"
#include <cctype>
#include <chrono>
#include <thread>
#include <stdexcept>
#include <initializer_list>
#include <ut.h>

namespace {

// Merge the same check across PLAYING phases: any failure fails it; otherwise
// a pass in any executed phase passes it.
CheckOutcome mergeOutcomes(std::initializer_list<CheckOutcome> outcomes)
{
    const auto rank = [](CheckOutcome outcome) {
        switch (outcome) {
            case CheckOutcome::FAILED: return 3;
            case CheckOutcome::PASSED: return 2;
            case CheckOutcome::SKIPPED: return 1;
            default: return 0;
        }
    };
    CheckOutcome merged = CheckOutcome::NOT_RUN;
    for (const CheckOutcome outcome : outcomes) {
        if (rank(outcome) > rank(merged)) {
            merged = outcome;
        }
    }
    return merged;
}

void reportCheck(const char* name, CheckOutcome outcome)
{
    switch (outcome) {
        case CheckOutcome::PASSED:
            CommonTestFramework::reportAssertion(name, VerificationResult::SUCCESS);
            break;
        case CheckOutcome::FAILED:
            CommonTestFramework::reportAssertion(name, VerificationResult::FAILURE);
            break;
        case CheckOutcome::SKIPPED:
            UT_LOG("[SKIP] %s: not applicable or unsupported", name);
            break;
        case CheckOutcome::NOT_RUN:
            UT_LOG("[NOT RUN] %s: test aborted before this check", name);
            break;
    }
}

// Returns false (after logging and failing the test) when the codec URL is not set.
bool codecUrlConfigured(const std::string& testName, const char* codec, std::string& url)
{
    const auto& codecUrls = CommonTestFramework::getConfiguration().codecUrls;
    const auto found = codecUrls.find(codec);
    if (found != codecUrls.end() && !found->second.empty()) {
        url = found->second;
        return true;
    }
    std::string key(codec);
    for (char& c : key) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    UT_LOG("In %s", testName.c_str());
    UT_LOG("ERROR: %s_url is not configured in [codecs] of platform.ini", key.c_str());
    CommonTestFramework::resetStatistics();
    CommonTestFramework::reportAssertion("Media URL Configured", VerificationResult::FAILURE);
    CommonTestFramework::printStatistics(testName);
    CommonTestFramework::resetStatistics();
    UT_LOG("Out %s", testName.c_str());
    return false;
}

CheckOutcome stateRequestFailed(const PipelineHandle& pipeline)
{
    return pipeline.busMonitor && pipeline.busMonitor->hasError() ?
        CheckOutcome::NOT_RUN : CheckOutcome::FAILED;
}

}

/**
 * @brief Test: Basic playback verification
 * @test_id: MP_BASIC_PLAY_001
 *
 * Flow: PLAYING -> continuous PLAYING for play_timeout_seconds -> NULL.
 * Uses the same [codecs] URLs as the play/pause suite. Asserts in order:
 *   PLAYING State, First Video Frame Signal -> after start
 *   4 "Advancing" checks                   -> after the PLAYING loop
 *   NULL State, No Bus Errors              -> at teardown
 * Video-only checks are skipped for audio-only pipelines.
 */
static void run_basic_playback(
    const std::string& testName,
    const std::string& url,
    PipelineMediaType mediaType = PipelineMediaType::AudioVideo)
{
    UT_LOG("In %s", testName.c_str());

    CommonTestFramework::resetStatistics();

    PhaseCheckResults playing;
    bool playingStateReported = false;
    bool firstFrameReported = false;
    bool metricsReported = false;
    bool startStatePassed = false;

    const auto reportPlayingState = [&](CheckOutcome outcome) {
        playingStateReported = true;
        startStatePassed = outcome == CheckOutcome::PASSED;
        reportCheck("PLAYING State", outcome);
    };
    const auto reportFirstFrame = [&](CheckOutcome outcome) {
        firstFrameReported = true;
        reportCheck("First Video Frame Signal", outcome);
    };
    const auto reportMetrics = [&]() {
        if (metricsReported) {
            return;
        }
        metricsReported = true;
        if (startStatePassed && playing.state == CheckOutcome::FAILED) {
            reportCheck("PLAYING State Maintained", CheckOutcome::FAILED);
        }
        reportCheck("Position Advancing", playing.position);
        reportCheck("Video PTS Advancing", playing.pts);
        reportCheck("Video Frames Advancing", playing.videoFrames);
        reportCheck("Audio Frames Advancing", playing.audioFrames);
    };
    const auto reportPending = [&]() {
        if (!playingStateReported) {
            reportPlayingState(CheckOutcome::NOT_RUN);
        }
        if (!firstFrameReported) {
            reportFirstFrame(CheckOutcome::NOT_RUN);
        }
        reportMetrics();
    };
    const auto reportSetupFailure = [](const std::exception& e) {
        UT_LOG("ERROR: %s", e.what());
        CommonTestFramework::reportAssertion(
            "Basic Playback Test Setup", VerificationResult::FAILURE);
    };

    try {
        const MediaPipelineConfiguration& config = CommonTestFramework::getConfiguration();
        UT_LOG("Media URL: %s", url.c_str());
        TestSetup test(config, url, mediaType);
        const PipelineHandle& pipeline = test.getPipeline();

        const auto runPhases = [&]() {
            const CheckOutcome state = PhaseValidation::verifyState(
                pipeline, GST_STATE_PLAYING, config.playTimeoutSeconds);
            reportPlayingState(state);
            if (state != CheckOutcome::PASSED) {
                return;
            }
            const CheckOutcome firstFrame =
                PhaseValidation::verifyFirstFrame(pipeline, config.playTimeoutSeconds);
            reportFirstFrame(firstFrame);
            if (firstFrame == CheckOutcome::FAILED || firstFrame == CheckOutcome::NOT_RUN) {
                return;
            }
            PhaseValidation::monitorPhase(
                pipeline, config.playTimeoutSeconds * 1000, false, false, playing);
            reportMetrics();
        };
        try {
            runPhases();
        }
        catch (const std::exception& e) {
            reportSetupFailure(e);
        }
        reportPending();
        reportCheck("NULL State", PhaseValidation::verifyNullState(pipeline));
        reportCheck("No Bus Errors", PhaseValidation::verifyNoBusErrors(pipeline));
    }
    catch (const std::exception& e) {
        reportSetupFailure(e);
        reportPending();
        reportCheck("NULL State", CheckOutcome::NOT_RUN);
        reportCheck("No Bus Errors", CheckOutcome::NOT_RUN);
    }

    CommonTestFramework::printStatistics(testName);
    CommonTestFramework::resetStatistics();
    UT_LOG("Out %s", testName.c_str());
}

static void run_codec_basic_playback(const char* codec, PipelineMediaType mediaType)
{
    const std::string testName = std::string("basic_playback_") + codec;
    std::string url;
    if (codecUrlConfigured(testName, codec, url)) {
        run_basic_playback(testName, url, mediaType);
    }
}

static void basic_playback_H264(void) { run_codec_basic_playback("H264", PipelineMediaType::AudioVideo); }
static void basic_playback_HEVC(void) { run_codec_basic_playback("HEVC", PipelineMediaType::AudioVideo); }
static void basic_playback_AV1(void) { run_codec_basic_playback("AV1", PipelineMediaType::AudioVideo); }
static void basic_playback_VP9(void) { run_codec_basic_playback("VP9", PipelineMediaType::AudioVideo); }
static void basic_playback_OPUS(void) { run_codec_basic_playback("OPUS", PipelineMediaType::AudioOnly); }
static void basic_playback_AAC(void) { run_codec_basic_playback("AAC", PipelineMediaType::AudioOnly); }
static void basic_playback_EAC3(void) { run_codec_basic_playback("EAC3", PipelineMediaType::AudioOnly); }

/**
 * @brief Test: Play/Pause pipeline operations
 * @test_id: MP_PLAY_PAUSE_001
 *
 * Flow: PLAYING for play_timeout_seconds -> PAUSED -> resume PLAYING
 *       -> continuous PLAYING for play_timeout_seconds -> NULL.
 * Reports each assertion as soon as its check completes:
 *   PLAYING State, First Video Frame Signal -> after start
 *   PAUSED State                           -> after pause request
 *   4 "Not Advancing" checks               -> after the paused monitoring loop
 *   4 "Advancing" checks                   -> after the final PLAYING loop
 *                                             (merged across start/resume/continuous)
 *   NULL State, No Bus Errors              -> at teardown
 * Any failure aborts the remaining phases; pending checks are then reported
 * (or logged as NOT RUN), and NULL state and bus error checks always run.
 */
static void run_play_pause_pipeline(
    const std::string& testName,
    const std::string& videoUrl,
    PipelineMediaType mediaType = PipelineMediaType::AudioVideo)
{
    UT_LOG("In %s", testName.c_str());

    CommonTestFramework::resetStatistics();

    PhaseCheckResults started, paused, resumed, continuous;
    bool playingStateReported = false;
    bool firstFrameReported = false;
    bool pausedStateReported = false;
    bool playingMetricsReported = false;
    bool pausedMetricsReported = false;
    bool startStatePassed = false;
    bool pausedStatePassed = false;

    const auto reportPlayingState = [&](CheckOutcome outcome) {
        playingStateReported = true;
        startStatePassed = outcome == CheckOutcome::PASSED;
        reportCheck("PLAYING State", outcome);
    };
    const auto reportFirstFrame = [&](CheckOutcome outcome) {
        firstFrameReported = true;
        reportCheck("First Video Frame Signal", outcome);
    };
    const auto reportPausedState = [&](CheckOutcome outcome) {
        pausedStateReported = true;
        pausedStatePassed = outcome == CheckOutcome::PASSED;
        reportCheck("PAUSED State", outcome);
    };
    const auto reportPausedMetrics = [&]() {
        if (pausedMetricsReported) {
            return;
        }
        pausedMetricsReported = true;
        // The state is re-checked on every poll; losing it is an extra failure.
        if (pausedStatePassed && paused.state == CheckOutcome::FAILED) {
            reportCheck("PAUSED State Maintained", CheckOutcome::FAILED);
        }
        reportCheck("Position Not Advancing", paused.position);
        reportCheck("Video PTS Not Advancing", paused.pts);
        reportCheck("Video Frames Not Advancing", paused.videoFrames);
        reportCheck("Audio Frames Not Advancing", paused.audioFrames);
    };
    const auto reportPlayingMetrics = [&]() {
        if (playingMetricsReported) {
            return;
        }
        playingMetricsReported = true;
        if ((startStatePassed && started.state == CheckOutcome::FAILED) ||
            resumed.state == CheckOutcome::FAILED || continuous.state == CheckOutcome::FAILED) {
            reportCheck("PLAYING State Maintained", CheckOutcome::FAILED);
        }
        reportCheck("Position Advancing",
            mergeOutcomes({started.position, resumed.position, continuous.position}));
        reportCheck("Video PTS Advancing",
            mergeOutcomes({started.pts, resumed.pts, continuous.pts}));
        reportCheck("Video Frames Advancing",
            mergeOutcomes({started.videoFrames, resumed.videoFrames, continuous.videoFrames}));
        reportCheck("Audio Frames Advancing",
            mergeOutcomes({started.audioFrames, resumed.audioFrames, continuous.audioFrames}));
    };
    const auto reportPending = [&]() {
        if (!playingStateReported) {
            reportPlayingState(CheckOutcome::NOT_RUN);
        }
        if (!firstFrameReported) {
            reportFirstFrame(CheckOutcome::NOT_RUN);
        }
        if (!pausedStateReported) {
            reportPausedState(CheckOutcome::NOT_RUN);
        }
        reportPausedMetrics();
        reportPlayingMetrics();
    };
    const auto reportSetupFailure = [](const std::exception& e) {
        UT_LOG("ERROR: %s", e.what());
        CommonTestFramework::reportAssertion(
            "Play/Pause Test Setup", VerificationResult::FAILURE);
    };

    try {
        const MediaPipelineConfiguration& config = CommonTestFramework::getConfiguration();
        UT_LOG("Media URL: %s", videoUrl.c_str());
        TestSetup test(config, videoUrl, mediaType);
        const PipelineHandle& pipeline = test.getPipeline();
        const unsigned int playMs = config.playTimeoutSeconds * 1000;

        const auto runPhases = [&]() {
            started.state = PhaseValidation::verifyState(
                pipeline, GST_STATE_PLAYING, config.playTimeoutSeconds);
            reportPlayingState(started.state);
            if (started.state != CheckOutcome::PASSED) {
                return;
            }
            const CheckOutcome firstFrame =
                PhaseValidation::verifyFirstFrame(pipeline, config.playTimeoutSeconds);
            reportFirstFrame(firstFrame);
            if (firstFrame == CheckOutcome::FAILED || firstFrame == CheckOutcome::NOT_RUN) {
                return;
            }
            if (PhaseValidation::monitorPhase(pipeline, playMs, false, false, started) !=
                VerificationResult::SUCCESS) {
                return;
            }

            const CheckOutcome pausedState = PipelineManagementLayer::setState_Paused(pipeline) ?
                PhaseValidation::verifyState(pipeline, GST_STATE_PAUSED,
                    config.pauseTimeoutSeconds) : stateRequestFailed(pipeline);
            reportPausedState(pausedState);
            if (pausedState != CheckOutcome::PASSED) {
                return;
            }
            const VerificationResult pausedResult = PhaseValidation::monitorPhase(
                pipeline, config.pauseTimeoutSeconds * 1000, true, false, paused);
            reportPausedMetrics();
            if (pausedResult != VerificationResult::SUCCESS) {
                return;
            }

            // Resume is part of the merged PLAYING checks; only a failure is asserted here.
            const CheckOutcome resumeState = PipelineManagementLayer::setState_Playing(pipeline) ?
                PhaseValidation::verifyState(pipeline, GST_STATE_PLAYING,
                    config.resumeTimeoutSeconds) : stateRequestFailed(pipeline);
            if (resumeState != CheckOutcome::PASSED) {
                reportCheck("PLAYING State (resume)", resumeState);
                return;
            }
            if (PhaseValidation::monitorPhase(pipeline, config.resumeTimeoutSeconds * 1000,
                    false, true, resumed) != VerificationResult::SUCCESS) {
                return;
            }
            PhaseValidation::monitorPhase(pipeline, playMs, false, false, continuous);
            reportPlayingMetrics();
        };
        try {
            runPhases();
        }
        catch (const std::exception& e) {
            reportSetupFailure(e);
        }
        reportPending();
        reportCheck("NULL State", PhaseValidation::verifyNullState(pipeline));
        reportCheck("No Bus Errors", PhaseValidation::verifyNoBusErrors(pipeline));
    }
    catch (const std::exception& e) {
        reportSetupFailure(e);
        reportPending();
        reportCheck("NULL State", CheckOutcome::NOT_RUN);
        reportCheck("No Bus Errors", CheckOutcome::NOT_RUN);
    }

    CommonTestFramework::printStatistics(testName);
    CommonTestFramework::resetStatistics();
    UT_LOG("Out %s", testName.c_str());
}

static void run_codec_play_pause(const char* codec, PipelineMediaType mediaType)
{
    const std::string testName = std::string("play_pause_") + codec;
    std::string url;
    if (codecUrlConfigured(testName, codec, url)) {
        run_play_pause_pipeline(testName, url, mediaType);
    }
}

static void play_pause_H264(void) { run_codec_play_pause("H264", PipelineMediaType::AudioVideo); }
static void play_pause_HEVC(void) { run_codec_play_pause("HEVC", PipelineMediaType::AudioVideo); }
static void play_pause_AV1(void) { run_codec_play_pause("AV1", PipelineMediaType::AudioVideo); }
static void play_pause_VP9(void) { run_codec_play_pause("VP9", PipelineMediaType::AudioVideo); }
static void play_pause_OPUS(void) { run_codec_play_pause("OPUS", PipelineMediaType::AudioOnly); }
static void play_pause_AAC(void) { run_codec_play_pause("AAC", PipelineMediaType::AudioOnly); }
static void play_pause_EAC3(void) { run_codec_play_pause("EAC3", PipelineMediaType::AudioOnly); }

/**
 * @brief Test: Frame validation
 * @test_id: MP_FRAME_VALID_001
 * 
 * Verifies:
 * - Frames are received in correct order
 * - Frame timestamps are monotonically increasing
 * - Frame dimensions match expected values
 * - Frame data integrity
 */
void test_frame_validation(void)
{
    UT_LOG("In %s", __FUNCTION__);
    
    try {
        UT_LOG("Test: Frame validation starting...");
        throw std::runtime_error("test_frame_validation not implemented");
    }
    catch (const std::exception& e) {
        UT_LOG("ERROR: %s", e.what());
    }
}

/**
 * @brief Test: Audio validation
 * @test_id: MP_AUDIO_VALID_001
 */
void test_audio_validation(void)
{
    UT_LOG("In %s", __FUNCTION__);
    
    try {
        UT_LOG("Test: Audio validation starting...");
        throw std::runtime_error("test_audio_validation not implemented");
    }
    catch (const std::exception& e) {
        UT_LOG("ERROR: %s", e.what());
    }
}

/**
 * @brief Test: Audio/Video synchronization
 * @test_id: MP_AV_SYNC_001
 * 
 * Verifies:
 * - Audio and video streams are in sync
 * - Lip-sync is maintained within acceptable threshold
 * - No audio/video drift over extended playback
 */
void test_av_sync(void)
{
    UT_LOG("In %s", __FUNCTION__);
    
    try {
        UT_LOG("Test: A/V sync starting...");
        throw std::runtime_error("test_av_sync not implemented");
    }
    catch (const std::exception& e) {
        UT_LOG("ERROR: %s", e.what());
    }
}

/**
 * @brief Test: Trickplay operations (fast forward, rewind, slow motion)
 * @test_id: MP_TRICK_PLAY_001
 */
void test_trickplay_operations(void)
{
    UT_LOG("In %s", __FUNCTION__);
    
    try {
        UT_LOG("Test: Trickplay operations starting...");
        throw std::runtime_error("test_trickplay_operations not implemented");
    }
    catch (const std::exception& e) {
        UT_LOG("ERROR: %s", e.what());
    }
}

/**
 * @brief Test: Buffer underflow handling
 * @test_id: MP_BUFFER_UNDER_001
 * 
 * Verifies:
 * - Pipeline handles buffer underflow gracefully
 * - Playback resumes when buffer is refilled
 * - Error callbacks are triggered appropriately
 */
void test_buffer_underflow(void)
{
    UT_LOG("In %s", __FUNCTION__);
    
    try {
        UT_LOG("Test: Buffer underflow starting...");
        throw std::runtime_error("test_buffer_underflow not implemented");
    }
    catch (const std::exception& e) {
        UT_LOG("ERROR: %s", e.what());
    }
}

/**
 * @brief Register all tests with the ut-core framework
 * 
 * This function is called from mediapipelinetests_main.cpp to register all tests
 */
void register_mp_tests(void)
{
    UT_LOG("Registering scenario suites...");
    const auto addSuite = [](const char* name) {
        UT_test_suite_t* suite = UT_add_suite(name, nullptr, nullptr);
        if (!suite) {
            UT_LOG("ERROR: Failed to create suite: %s", name);
            throw std::runtime_error(std::string("Failed to create suite: ") + name);
        }
        return suite;
    };
    const auto addTest = [](UT_test_suite_t* suite, const char* name, UT_TestFunction_t function) {
        if (!UT_add_test(suite, name, function)) {
            UT_LOG("ERROR: Failed to register test: %s", name);
            throw std::runtime_error(std::string("Failed to register test: ") + name);
        }
    };

    struct CodecTest {
        const char* name;
        UT_TestFunction_t function;
    };
    // Every codec case is always listed; a missing URL fails the case when it runs.
    const auto addCodecSuite = [&](const char* suiteName,
        std::initializer_list<CodecTest> codecTests) {
        UT_test_suite_t* suite = addSuite(suiteName);
        for (const CodecTest& codecTest : codecTests) {
            addTest(suite, codecTest.name, codecTest.function);
        }
    };

    addCodecSuite("test_basic_playback", {
        {"basic_playback_H264", basic_playback_H264},
        {"basic_playback_HEVC", basic_playback_HEVC},
        {"basic_playback_AV1", basic_playback_AV1},
        {"basic_playback_VP9", basic_playback_VP9},
        {"basic_playback_OPUS", basic_playback_OPUS},
        {"basic_playback_AAC", basic_playback_AAC},
        {"basic_playback_EAC3", basic_playback_EAC3}
    });

    addCodecSuite("test_play_pause_pipeline", {
        {"play_pause_H264", play_pause_H264},
        {"play_pause_HEVC", play_pause_HEVC},
        {"play_pause_AV1", play_pause_AV1},
        {"play_pause_VP9", play_pause_VP9},
        {"play_pause_OPUS", play_pause_OPUS},
        {"play_pause_AAC", play_pause_AAC},
        {"play_pause_EAC3", play_pause_EAC3}
    });

    addTest(addSuite("test_frame_validation"), "not_implemented", test_frame_validation);
    addTest(addSuite("test_audio_validation"), "not_implemented", test_audio_validation);
    addTest(addSuite("test_av_sync"), "not_implemented", test_av_sync);
    addTest(addSuite("test_trickplay_operations"), "not_implemented", test_trickplay_operations);
    addTest(addSuite("test_buffer_underflow"), "not_implemented", test_buffer_underflow);

    UT_LOG("Scenario suites registered. Ready to execute.");
}
