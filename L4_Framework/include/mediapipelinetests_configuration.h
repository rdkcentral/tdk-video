/**
 * @file mediapipelinetests_configuration.h
 * @brief Configuration management for MediaPipelineTests framework
 * 
 * This file defines configuration structures and interfaces for loading
 * test parameters from platform.ini file instead of command-line arguments
 */

#ifndef MP_CONFIGURATION_H
#define MP_CONFIGURATION_H

#include <string>
#include <map>
#include <gst/gst.h>
/**
 * @struct MediaPipelineConfiguration
 * @brief Type-safe configuration container for all test parameters
 * 
 * All test parameters are externalized here. Tests use these values
 * instead of global variables or command-line arguments.
 */
struct MediaPipelineConfiguration
{
    // Platform identification
    std::string platform;
    
    // Video URLs for different scenarios
    std::string testUrl;
    std::string pauseVideo;
    std::string trickplayVideo;
    std::string bufferUnderflowVideo;
    std::map<std::string, std::string> codecUrls;
    
    // GStreamer element names (platform-specific)
    std::string sourceElement;
    std::string decoderElement;
    std::string audioSink;
    std::string videoSink;
    std::string audioFilter;
    std::string videoFilter;
    std::string playbinVideoFlags;
    std::string playbinAudioFlags;
    
    // Timeout thresholds (seconds)
    unsigned int playTimeoutSeconds;
    unsigned int pauseTimeoutSeconds;
    unsigned int resumeTimeoutSeconds;
    unsigned int eosTimeoutSeconds;
    unsigned int positionToleranceMs;
    unsigned int pausePositionToleranceMs;
    unsigned int sinkStallTimeoutMs;
    
    // Frame drop and sync tolerances
    unsigned int allowedFrameDrops;
    unsigned int consecutiveFrameDrops;
    unsigned int ptsErrorToleranceMs;
    unsigned int avSyncToleranceMs;

    // Sink metric validation: a check fails on its Nth violation in one phase
    bool videoFramesValidationEnabled = true;
    bool audioFramesValidationEnabled = true;
    unsigned int ptsErrorTolerance = 1;
    unsigned int videoFramesTolerance = 1;
    unsigned int audioFramesTolerance = 1;
    
    // Video/Audio validation thresholds
    unsigned int minFramesExpected;
    unsigned int samplingRate;
    unsigned int bitDepth;
    
    // Feature flags
    bool checkAvStatus;
    bool checkPts;
    bool failOnBusError = true;
    bool useWesterossinkFps;
    bool enableDotGraphs;
    bool enableLogging;
    std::string logLevel;
    unsigned int verificationLogIntervalMs = 1000;
    
    // Media configuration
    double volume;
    bool mute;
    unsigned int bitrate;
};

/**
 * @class ConfigurationLayer
 * @brief Loads and parses INI configuration files
 * 
 * Reads platform.ini and converts key/value pairs into a type-safe
 * configuration structure with proper validation.
 */
class ConfigurationLayer
{
public:
    /**
     * @brief Load configuration from INI file
     * @param path Path to the platform.ini configuration file
     * @param config Output configuration structure
     * @return True if successful, false otherwise
     * 
     * TODO: Implement INI file parsing
     * - Read file content
     * - Parse [section] headers
     * - Parse key=value pairs
     * - Handle comments (# and ;)
     * - Convert string values to appropriate types
     * - Validate required keys are present
     * - Apply type conversions and defaults
     */
    static bool load(const std::string& path, MediaPipelineConfiguration& config);
    
private:
    /**
     * @brief Parse INI format text into configuration
     * @param text Raw INI file content
     * @param config Output configuration structure
     * @return True if successful, false otherwise
     * 
     * TODO: Implement line-by-line parsing
     * - Skip empty lines
     * - Skip lines starting with # or ;
     * - Extract key=value pairs
     * - Handle whitespace trimming
     * - Ignore [section] headers
     */
    static bool parseIni(const std::string& text, MediaPipelineConfiguration& config);
    
    /**
     * @brief Read file content
     * @param path File path to read
     * @param content Output file content
     * @return True if successful, false otherwise
     * 
     * TODO: Implement file reading
     * - Open file
     * - Read all content
     * - Close file
     * - Handle I/O errors
     */
    static bool readText(const std::string& path, std::string& content);
    
    /**
     * @brief Trim whitespace from string
     * @param value Input string
     * @return String with leading/trailing whitespace removed
     * 
     * TODO: Implement string trimming
     * - Remove leading whitespace
     * - Remove trailing whitespace
     * - Preserve interior whitespace
     */
    static std::string trim(const std::string& value);
    
    /**
     * @brief Get required key from config map
     * @param key Key to retrieve
     * @param section Section name (optional)
     * @return Value associated with key, or empty string if not found
     * 
     * TODO: Implement safe map access
     * - Check if key exists
     * - Return value if found
     * - Return empty string if not found
     */
    static std::string valueFor(
        const std::string& key,
        const std::string& section = "");
};

#endif // MP_CONFIGURATION_H
