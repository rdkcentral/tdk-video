#ifndef MP_ENVIRONMENT_H
#define MP_ENVIRONMENT_H

#include <optional>
#include <string>
#include <vector>
#include <sys/types.h>

class EnvironmentSetup
{
public:
    EnvironmentSetup() = default;
    ~EnvironmentSetup();
    EnvironmentSetup(const EnvironmentSetup&) = delete;
    EnvironmentSetup& operator=(const EnvironmentSetup&) = delete;

    // Throws on invalid configuration or a failed environment, library, or command operation.
    void initialize(const std::string& path);

private:
    struct SavedVariable
    {
        std::string name;
        std::optional<std::string> value;
    };

    std::vector<SavedVariable> m_savedVariables;
    std::vector<void*> m_libraries;
    std::vector<pid_t> m_backgroundProcesses;
};

#endif
