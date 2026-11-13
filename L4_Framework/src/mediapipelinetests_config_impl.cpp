/**
 * @file mediapipelinetests_config_impl.cpp
 * @brief Configuration layer - INI file parsing
 */

#include "mediapipelinetests_configuration.h"
#include "ut_log_wrapper.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <map>

namespace {

bool parseBool(const std::string& key, const std::string& value, bool& result)
{
    if (value != "true" && value != "false") {
        UT_LOG("ERROR: %s must be true or false", key.c_str());
        return false;
    }
    result = value == "true";
    return true;
}

bool parseTolerance(const std::string& key, const std::string& value, unsigned int& result)
{
    if (value.empty() || value.size() > 6 ||
        value.find_first_not_of("0123456789") != std::string::npos || std::stoul(value) == 0) {
        UT_LOG("ERROR: %s must be an integer between 1 and 999999", key.c_str());
        return false;
    }
    result = static_cast<unsigned int>(std::stoul(value));
    return true;
}

}

// ============================================================================
// String Utility Functions
// ============================================================================

std::string ConfigurationLayer::trim(const std::string& str)
{
    // Trim leading whitespace
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    
    // Trim trailing whitespace
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

// ============================================================================
// Configuration Loading
// ============================================================================

bool ConfigurationLayer::readText(
    const std::string& filePath,
    std::string& content)
{
    std::ifstream file(filePath);
    if (!file.is_open()) {
        UT_LOG("ERROR: Could not open configuration file: %s", filePath.c_str());
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    content = buffer.str();
    file.close();
    
    UT_LOG("Configuration file loaded: %s (%zu bytes)", filePath.c_str(), content.length());
    return true;
}

bool ConfigurationLayer::parseIni(
    const std::string& content,
    MediaPipelineConfiguration& config)
{
    std::istringstream stream(content);
    std::string line;
    std::string currentSection;
    
    while (std::getline(stream, line)) {
        // Remove comments
        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }
        
        // Trim whitespace
        line = trim(line);
        
        // Skip empty lines
        if (line.empty()) {
            continue;
        }
        
        // Check for section headers [section_name]
        if (line[0] == '[' && line[line.length() - 1] == ']') {
            currentSection = line.substr(1, line.length() - 2);
            continue;
        }
        
        // Parse key=value pairs
        size_t eqPos = line.find('=');
        if (eqPos == std::string::npos) {
            continue;  // Skip lines without '='
        }
        
        std::string key = trim(line.substr(0, eqPos));
        std::string value = trim(line.substr(eqPos + 1));
        
        // Process based on section
        if (currentSection == "platform") {
            if (key == "platform") {
                config.platform = value;
            }
            else if (key == "test_url") {
                config.testUrl = value;
            }
            else if (key == "pause_video") {
                config.pauseVideo = value;
            }
        }
        else if (currentSection == "codecs") {
            static const std::map<std::string, std::string> codecKeys = {
                {"h264_url", "H264"}, {"hevc_url", "HEVC"}, {"av1_url", "AV1"},
                {"vp9_url", "VP9"}, {"opus_url", "OPUS"}, {"aac_url", "AAC"},
                {"eac3_url", "EAC3"}
            };
            const auto codec = codecKeys.find(key);
            if (codec == codecKeys.end()) {
                UT_LOG("ERROR: Unsupported codec URL key: %s", key.c_str());
                return false;
            }
            if (!config.codecUrls.emplace(codec->second, value).second) {
                UT_LOG("ERROR: Duplicate codec URL: %s", key.c_str());
                return false;
            }
        }
        else if (currentSection == "elements") {
            if (key == "playbin_flags") {
                UT_LOG("ERROR: playbin_flags is replaced by playbin_video_flags and playbin_audio_flags");
                return false;
            }
            else if (key == "playbin_video_flags" || key == "playbin_audio_flags") {
                if (value.empty()) {
                    UT_LOG("ERROR: %s cannot be empty; omit the key instead", key.c_str());
                    return false;
                }
                (key == "playbin_video_flags" ? config.playbinVideoFlags :
                    config.playbinAudioFlags) = value;
            }
            else if (key == "audio_sink") {
                config.audioSink = value;
            }
            else if (key == "video_sink") {
                config.videoSink = value;
            }
            else if (key == "decoder_element") {
                config.decoderElement = value;
            }
        }
        else if (currentSection == "thresholds") {
            if (key == "position_tolerance_ms") {
                config.positionToleranceMs = std::stoul(value);
            }
            else if (key == "pause_position_tolerance_ms") {
                config.pausePositionToleranceMs = std::stoul(value);
            }
            else if (key == "sink_stall_timeout_ms") {
                config.sinkStallTimeoutMs = std::stoul(value);
            }
            else if (key == "play_timeout_seconds") {
                config.playTimeoutSeconds = std::stoul(value);
            }
            else if (key == "pause_timeout_seconds") {
                config.pauseTimeoutSeconds = std::stoul(value);
            }
            else if (key == "resume_timeout_seconds") {
                config.resumeTimeoutSeconds = std::stoul(value);
            }
            else if (key == "eos_timeout_seconds") {
                config.eosTimeoutSeconds = std::stoul(value);
            }
            else if (key == "allowed_frame_drops") {
                config.allowedFrameDrops = std::stoul(value);
            }
            else if (key == "av_sync_tolerance_ms") {
                config.avSyncToleranceMs = std::stoll(value);
            }
        }
        else if (currentSection == "validation") {
            bool valid = true;
            if (key == "pts_validation_enabled") {
                valid = parseBool(key, value, config.checkPts);
            }
            else if (key == "video_frames_validation_enabled") {
                valid = parseBool(key, value, config.videoFramesValidationEnabled);
            }
            else if (key == "audio_frames_validation_enabled") {
                valid = parseBool(key, value, config.audioFramesValidationEnabled);
            }
            else if (key == "pts_error_tolerance") {
                valid = parseTolerance(key, value, config.ptsErrorTolerance);
            }
            else if (key == "video_frames_tolerance") {
                valid = parseTolerance(key, value, config.videoFramesTolerance);
            }
            else if (key == "audio_frames_tolerance") {
                valid = parseTolerance(key, value, config.audioFramesTolerance);
            }
            else {
                UT_LOG("ERROR: Unsupported validation key: %s", key.c_str());
                valid = false;
            }
            if (!valid) {
                return false;
            }
        }
        else if (currentSection == "features") {
            if (key == "verification_log_interval_ms") {
                if (value.empty() || value.find_first_not_of("0123456789") != std::string::npos) {
                    UT_LOG("ERROR: verification_log_interval_ms must be an integer >= 100");
                    return false;
                }
                const unsigned long interval = std::stoul(value);
                if (interval < 100 || interval > 3600000) {
                    UT_LOG("ERROR: verification_log_interval_ms must be between 100 and 3600000");
                    return false;
                }
                config.verificationLogIntervalMs = static_cast<unsigned int>(interval);
            }
            else if (key == "log_level") {
                if (value != "info" && value != "debug") {
                    UT_LOG("ERROR: log_level must be info or debug");
                    return false;
                }
                config.logLevel = value;
            }
            else if (key == "fail_on_bus_error") {
                if (value != "true" && value != "false") {
                    UT_LOG("ERROR: fail_on_bus_error must be true or false");
                    return false;
                }
                config.failOnBusError = value == "true";
            }
            else if (key == "check_av_status") {
                config.checkAvStatus = (value == "true");
            }
            else if (key == "check_pts") {
                config.checkPts = (value == "true");
            }
            else if (key == "enable_logging") {
                config.enableLogging = (value == "true");
            }
        }
        else if (currentSection == "media") {
            if (key == "volume") {
                config.volume = std::stof(value);
            }
            else if (key == "mute") {
                config.mute = (value == "true");
            }
        }
    }
    
    return true;
}

std::string ConfigurationLayer::valueFor(
    const std::string& key,
    const std::string& section)
{
    // This is a simplified stub - values are set during parseIni in load()
    // Returns empty string for lookups - actual values come from parsing INI file
    return "";
}

bool ConfigurationLayer::load(
    const std::string& configPath,
    MediaPipelineConfiguration& config)
{
    UT_LOG("Loading configuration from: %s", configPath.c_str());
    
    // Platform values and media URLs must come from the configuration file.
    config = {};
    config.playTimeoutSeconds = 10;
    config.pauseTimeoutSeconds = 5;
    config.resumeTimeoutSeconds = 5;
    config.eosTimeoutSeconds = 120;
    config.positionToleranceMs = 900;
    config.pausePositionToleranceMs = 100;
    config.sinkStallTimeoutMs = 2000;
    config.audioSink = "autoaudiosink";
    config.videoSink = "autovideosink";
    config.allowedFrameDrops = 5;
    config.avSyncToleranceMs = 100;
    config.checkAvStatus = true;
    config.checkPts = true;
    config.enableLogging = true;
    config.logLevel = "info";
    config.volume = 1.0f;
    config.mute = false;
    
    // Read and parse configuration file
    std::string content;
    if (!readText(configPath, content)) {
        return false;
    }
    
    if (!parseIni(content, config)) {
        UT_LOG("ERROR: Failed to parse configuration file");
        return false;
    }
    if (config.playTimeoutSeconds == 0 || config.pauseTimeoutSeconds == 0 ||
        config.resumeTimeoutSeconds == 0 || config.sinkStallTimeoutMs == 0) {
        UT_LOG("ERROR: Playback, pause, resume and sink stall timeouts must be positive");
        return false;
    }
    
    UT_LOG("Configuration successfully loaded and parsed");
    return true;
}
