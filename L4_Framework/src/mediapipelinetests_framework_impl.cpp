/**
 * @file mediapipelinetests_framework_impl.cpp
 * @brief Framework implementation with assertion counting
 * 
 * CRITICAL: This implements the assertion counting logic that was
 * requested - each verification = 1 assertion
 */

#include "mediapipelinetests_common_framework.h"
#include "mediapipelinetests_configuration.h"
#include "mediapipelinetests_pipeline_management.h"
#include "mediapipelinetests_environment.h"
#include "ut_log_wrapper.h"
#include <cstdio>
#include <cstdarg>
#include <stdexcept>
#include <filesystem>
#include <memory>

// Global statistics and configuration
static TestStatistics g_statistics {};  // C++11 aggregate initialization
static MediaPipelineConfiguration g_configuration;
static bool g_initialized = false;
static std::unique_ptr<EnvironmentSetup> g_environment;

// ============================================================================
// CommonTestFramework Implementation
// ============================================================================

bool CommonTestFramework::initialize(
    const std::string& configPath,
    const std::string& environmentPath)
{
    UT_LOG("Initializing MediaPipelineTests framework...");
    if (g_initialized) {
        UT_LOG("ERROR: Framework is already initialized");
        return false;
    }
    
    // Load configuration from file
    if (!ConfigurationLayer::load(configPath, g_configuration)) {
        UT_LOG("ERROR: Failed to load configuration from %s", configPath.c_str());
        return false;
    }

    const std::string envPath = environmentPath.empty() ?
        (std::filesystem::path(configPath).parent_path() / "env.yaml").string() :
        environmentPath;
    auto environment = std::make_unique<EnvironmentSetup>();
    environment->initialize(envPath);

    GError* error = nullptr;
    if (!gst_init_check(nullptr, nullptr, &error)) {
        UT_LOG("ERROR: GStreamer initialization failed: %s",
            error ? error->message : "unknown error");
        if (error) {
            g_error_free(error);
        }
        return false;
    }
    g_environment = std::move(environment);
    
    UT_LOG("Configuration loaded:");
    UT_LOG("Platform: %s", g_configuration.platform.c_str());
    UT_LOG("Play Timeout: %u seconds", g_configuration.playTimeoutSeconds);
    UT_LOG("Pause Timeout: %u seconds", g_configuration.pauseTimeoutSeconds);
    
    g_initialized = true;
    UT_LOG("Framework initialized successfully");
    return true;
}

void CommonTestFramework::shutdown()
{
    if (g_initialized) {
        gst_deinit();
        g_initialized = false;
    }
    g_environment.reset();
}

const MediaPipelineConfiguration& CommonTestFramework::getConfiguration()
{
    if (!g_initialized) {
        throw std::runtime_error("Framework not initialized");
    }
    return g_configuration;
}

bool CommonTestFramework::reportAssertion(
    const std::string& name,
    VerificationResult result)
{
    // CRITICAL: Increment total assertion counter
    g_statistics.totalAssertions++;
    
    bool passed = false;
    
    if (result == VerificationResult::SUCCESS) {
        g_statistics.passedAssertions++;
        UT_LOG("[ASSERT] PASS: %s", name.c_str());
        passed = true;
        
        // Call ut-core UT_ASSERT_EQUAL to integrate with framework
        UT_ASSERT_EQUAL(
            static_cast<int>(result),
            static_cast<int>(VerificationResult::SUCCESS));
    }
    else {
        g_statistics.failedAssertions++;
        UT_LOG("[ASSERT] FAIL: %s (result=%d)", name.c_str(), (int)result);
        passed = false;
        
        // Still call UT_ASSERT to register failure with ut-core
        UT_ASSERT_EQUAL(
            static_cast<int>(result),
            static_cast<int>(VerificationResult::SUCCESS));
    }
    
    return passed;
}

bool CommonTestFramework::reportAssertionWithDetail(
    const std::string& name,
    VerificationResult result,
    const std::string& detail)
{
    g_statistics.totalAssertions++;
    
    bool passed = (result == VerificationResult::SUCCESS);
    
    if (passed) {
        g_statistics.passedAssertions++;
        UT_LOG("[ASSERT] PASS: %s - %s", name.c_str(), detail.c_str());
    }
    else {
        g_statistics.failedAssertions++;
        UT_LOG("[ASSERT] FAIL: %s - %s", name.c_str(), detail.c_str());
    }
    
    UT_ASSERT_EQUAL(
        static_cast<int>(result),
        static_cast<int>(VerificationResult::SUCCESS));
    return passed;
}

TestStatistics CommonTestFramework::getStatistics()
{
    return g_statistics;
}

void CommonTestFramework::printStatistics(const std::string& testName)
{
    const char* result = g_statistics.totalAssertions == 0 ? "NO ASSERTIONS" :
        (g_statistics.failedAssertions == 0 ? "PASS" : "FAIL");
    UT_LOG("[RESULT] %s: %s | assertions=%u passed=%u failed=%u",
        testName.c_str(), result, g_statistics.totalAssertions,
        g_statistics.passedAssertions, g_statistics.failedAssertions);
}

void CommonTestFramework::resetStatistics()
{
    g_statistics.totalAssertions = 0;
    g_statistics.passedAssertions = 0;
    g_statistics.failedAssertions = 0;
}

void CommonTestFramework::logMessage(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    fprintf(stdout, "[UT] ");
    vfprintf(stdout, format, args);
    fflush(stdout);
    va_end(args);
}

// ============================================================================
// TestSetup RAII Implementation
// ============================================================================

TestSetup::TestSetup(
    const MediaPipelineConfiguration& config,
    const std::string& videoUrl,
    PipelineMediaType mediaType)
    : m_pipeline{}
{
    UT_LOG("TestSetup: Initializing test infrastructure");
    
    if (!gst_is_initialized()) {
        throw std::runtime_error("GStreamer must be initialized before creating a test pipeline");
    }
    
    // Create the pipeline
    m_pipeline = PipelineManagementLayer::createPipeline(config, videoUrl, mediaType);
    
    // Set to PLAYING state
    if (!PipelineManagementLayer::setState_Playing(m_pipeline)) {
        PipelineManagementLayer::destroyPipeline(m_pipeline);
        throw std::runtime_error("Could not start test pipeline");
    }
    UT_LOG("TestSetup: Pipeline ready for testing");
}

TestSetup::~TestSetup()
{
    UT_LOG("TestSetup: Cleaning up test infrastructure");
    
    PipelineManagementLayer::destroyPipeline(m_pipeline);
    
    UT_LOG("TestSetup: Cleanup complete");
}

const PipelineHandle& TestSetup::getPipeline() const
{
    return m_pipeline;
}
