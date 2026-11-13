/**
 * @file mediapipelinetests_common_framework.h
 * @brief Common test framework and utilities
 * 
 * Provides shared infrastructure for all tests:
 * - Configuration loading
 * - Pipeline management
 * - Test execution harness
 * - Statistics collection
 */

#ifndef MP_COMMON_FRAMEWORK_H
#define MP_COMMON_FRAMEWORK_H

#include "mediapipelinetests_configuration.h"
#include "mediapipelinetests_pipeline_management.h"
#include "mediapipelinetests_verification.h"
#include <string>
#include <vector>

/**
 * @struct TestStatistics
 * @brief Counters for test execution statistics
 * 
 * Tracks:
 * - Total assertions executed
 * - Passing assertions
 * - Failing assertions
 * 
 * Each verification counts as ONE assertion, regardless of internal loop iterations
 */
struct TestStatistics
{
    unsigned int totalAssertions;    ///< Total assertions (each verification = 1)
    unsigned int passedAssertions;   ///< Assertions that passed
    unsigned int failedAssertions;   ///< Assertions that failed
    
    TestStatistics()
        : totalAssertions(0), passedAssertions(0), failedAssertions(0)
    {
    }
};

/**
 * @class CommonTestFramework
 * @brief Shared test execution infrastructure
 * 
 * Provides:
 * - Centralized configuration loading
 * - Test setup/teardown helpers
 * - Assertion reporting to ut-core
 * - Statistics collection
 */
class CommonTestFramework
{
public:
    /**
     * @brief Initialize test framework
     * @param configPath Path to platform.ini configuration file
     * @param environmentPath Path to env.yaml; defaults to beside platform.ini
     * @return true if initialization successful, false otherwise
     * 
     * TODO: Implement framework initialization
     * Steps:
     *   1. Load configuration from configPath
     *   2. Initialize ut-core logging
     *   3. Create configuration singleton
     *   4. Return success status
     * 
     * Called once per test suite execution
     */
    static bool initialize(
        const std::string& configPath,
        const std::string& environmentPath = "");

    /**
     * @brief Deinitialize GStreamer and release loaded environment resources.
     * Call after all test pipelines have been destroyed.
     */
    static void shutdown();
    
    /**
     * @brief Get loaded configuration
     * @return Reference to global configuration
     * 
     * TODO: Return singleton configuration instance
     * - Must be called after initialize()
     * - Returns configuration loaded from INI file
     */
    static const MediaPipelineConfiguration& getConfiguration();
    
    /**
     * @brief Report single assertion result to ut-core
     * @param verificationName Name of verification (for logging)
     * @param result VerificationResult (SUCCESS or FAILURE)
     * @return true if reported successfully
     * 
     * TODO: Implement assertion reporting
     * Steps:
     *   1. Increment totalAssertions counter
     *   2. If result == SUCCESS:
     *      - Call UT_ASSERT_PASS() or similar
     *      - Increment passedAssertions
     *      - Log to UT_LOG: "PASS: <verificationName>"
     *   3. If result == FAILURE:
     *      - Call UT_ASSERT() with false
     *      - Increment failedAssertions
     *      - Log to UT_LOG: "FAIL: <verificationName>"
     *   4. If result == TIMEOUT:
     *      - Report as failure (timeout = test failure)
     *      - Log timeout message
     *   5. Update global statistics
     * 
     * Returns: true if assertion passed, false if failed
     * Note: This is the key mechanism for counting assertions
     *       Each verification (even with internal loops) = 1 call here
     */
    static bool reportAssertion(
        const std::string& verificationName,
        VerificationResult result);
    
    /**
     * @brief Report assertion with detailed message
     * @param verificationName Name of verification
     * @param result VerificationResult
     * @param detailMessage Additional details for logging
     * @return true if assertion passed
     * 
     * TODO: Implement detailed assertion reporting
     * Similar to reportAssertion() but includes detail message
     * - Log format: "PASS/FAIL: <name> - <detail>"
     * - Update same counters as reportAssertion()
     * - Useful for logging observed vs expected values
     */
    static bool reportAssertionWithDetail(
        const std::string& verificationName,
        VerificationResult result,
        const std::string& detailMessage);
    
    /**
     * @brief Get current test statistics
     * @return TestStatistics struct with current counts
     * 
     * TODO: Return global statistics
     * - Total assertions (each verification call)
     * - Passed assertions
     * - Failed assertions
     * 
     * Called at end of test to report results
     */
    static TestStatistics getStatistics();
    
    /**
     * @brief Print test statistics summary
     * @param testName Name of test that completed
     * 
     * TODO: Implement statistics printing
     * Steps:
     *   1. Get current statistics
     *   2. Calculate pass percentage
     *   3. Print formatted summary using UT_LOG:
     *      Test: <testName>
     *      Total Assertions: X
     *      Passed: Y
     *      Failed: Z
     *      Success Rate: XX%
     */
    static void printStatistics(const std::string& testName);
    
    /**
     * @brief Reset statistics for next test
     * 
     * TODO: Implement statistics reset
     * - Clear counters
     * - Reset to 0/0/0
     * 
     * Called between tests
     */
    static void resetStatistics();
    
    /**
     * @brief Print log message
     * @param format Printf-style format string
     * @param ... Variable arguments
     * 
     * TODO: Implement logging
     * - Wrapper around ut-core UT_LOG
     * - Consistent formatting for MediaPipelineTests
     * - Include function name and line number if available
     */
    static void logMessage(const char* format, ...);
};

/**
 * @class TestSetup
 * @brief RAII helper for test setup/teardown
 * 
 * Automatically handles pipeline creation and destruction
 */
class TestSetup
{
public:
    /**
     * @brief Setup test with pipeline
     * @param config Configuration for pipeline
     * @param videoUrl URL to test video
     * 
     * TODO: Implement test setup
     * Steps:
     *   1. Create pipeline using PipelineManagementLayer
     *   2. Set to PLAYING state
     *   3. Store pipeline handle
     */
    TestSetup(const MediaPipelineConfiguration& config, const std::string& videoUrl,
        PipelineMediaType mediaType = PipelineMediaType::AudioVideo);
    
    /**
     * @brief Cleanup on destruction (RAII pattern)
     * 
     * TODO: Implement cleanup
     * - Destroy pipeline
     * - Release resources
     * - Called automatically on scope exit
     */
    ~TestSetup();
    TestSetup(const TestSetup&) = delete;
    TestSetup& operator=(const TestSetup&) = delete;
    
    /**
     * @brief Get pipeline handle
     * @return Reference to pipeline created in setup
     * 
     * TODO: Return stored pipeline handle
     */
    const PipelineHandle& getPipeline() const;
    
private:
    PipelineHandle m_pipeline;
};

#endif // MP_COMMON_FRAMEWORK_H
