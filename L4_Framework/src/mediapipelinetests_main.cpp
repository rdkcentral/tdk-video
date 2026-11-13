/**
 * @file mediapipelinetests_main.cpp
 * @brief Test suite entry point using ut-core framework
 * 
 * Initializes ut-core, registers tests, and runs test suite.
 */

#include "ut_log_wrapper.h"
#include "mediapipelinetests_common_framework.h"
extern "C" {
   #include <ut.h>
}
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

// Forward declarations
extern void register_mp_tests(void);

int main(int argc, char* argv[])
{

    UT_LOG("========================================");
    UT_LOG("MediaPipelineTests Framework");
    UT_LOG("========================================");
    
    std::string configPath;
    std::string environmentPath;
    std::vector<char*> utArguments{argv[0]};
    try {
        for (int i = 1; i < argc; ++i) {
            const std::string argument = argv[i];
            if (argument == "--config" || argument == "--env") {
                if (i + 1 >= argc) {
                    throw std::runtime_error("Missing path for " + argument);
                }
                if (argument == "--config") {
                    configPath = argv[++i];
                } else {
                    environmentPath = argv[++i];
                }
            } else if (i == 1 && !argument.empty() && argument[0] != '-') {
                configPath = argument;
            } else {
                utArguments.push_back(argv[i]);
            }
        }
        if (configPath.empty()) {
            for (const char* candidate : {"./config/platform.ini", "./platform.ini",
                     MP_DEFAULT_CONFIG_PATH}) {
                if (std::filesystem::is_regular_file(candidate)) {
                    configPath = candidate;
                    break;
                }
            }
            if (configPath.empty()) {
                throw std::runtime_error("No platform.ini found; specify --config /path/platform.ini");
            }
        }
    } catch (const std::exception& e) {
        UT_LOG("FATAL: %s", e.what());
        return 1;
    }

    const int utArgumentCount = static_cast<int>(utArguments.size());
    utArguments.push_back(nullptr);
    if (UT_init(utArgumentCount, utArguments.data()) != UT_STATUS_OK) {
        UT_LOG("FATAL: ut-core initialization failed");
        return 1;
    }

    try {
        if (!CommonTestFramework::initialize(configPath, environmentPath)) {
            UT_LOG("FATAL: Framework initialization failed for config: %s", configPath.c_str());
            CommonTestFramework::shutdown();
            UT_exit();
            return 1;
        }
    }
    catch (const std::exception& e) {
        UT_LOG("FATAL: Framework initialization failed: %s", e.what());
        CommonTestFramework::shutdown();
        UT_exit();
        return 1;
    }
    
    // Register all tests
    try {
        register_mp_tests();
    } catch (const std::exception& e) {
        UT_LOG("FATAL: Test registration failed: %s", e.what());
        CommonTestFramework::shutdown();
        UT_exit();
        return 1;
    }
    
    // Run all tests
    const UT_status_t status = UT_run_tests();
    
    // Cleanup
    UT_exit();
    CommonTestFramework::shutdown();
    
    return status == UT_STATUS_OK ? 0 : 1;
}
