#include "mediapipelinetests_environment.h"
#include "ut_log_wrapper.h"
#include <libfyaml.h>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <sys/wait.h>
#include <utility>
#include <fstream>
#include <deque>
#include <algorithm>
#include <fcntl.h>
#include <signal.h>
#include <spawn.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>

extern char** environ;

namespace {

struct Command
{
    std::string text;
    bool background = false;
    std::string socketPath;
    unsigned int timeoutSeconds = 3;
};

std::string scalar(fy_node* node, const std::string& field)
{
    if (!node || !fy_node_is_scalar(node) || fy_node_is_alias(node) ||
        fy_node_is_null(node)) {
        throw std::runtime_error(field + " must be a non-null YAML scalar");
    }
    size_t length = 0;
    const char* value = fy_node_get_scalar(node, &length);
    if (!value) {
        throw std::runtime_error("Could not read YAML scalar: " + field);
    }
    std::string text(value, length);
    size_t tagLength = 0;
    const char* tag = fy_node_get_tag(node, &tagLength);
    const std::string tagText = tag ? std::string(tag, tagLength) : "";
    if (tagText == "tag:yaml.org,2002:null" ||
        (tagText.empty() && fy_node_get_style(node) == FYNS_PLAIN &&
            (text.empty() || text == "~" || text == "null" ||
                text == "Null" || text == "NULL"))) {
        throw std::runtime_error(field + " must not be YAML null; quote literal strings");
    }
    if (text.find('\0') != std::string::npos) {
        throw std::runtime_error(field + " cannot contain a NUL character");
    }
    return text;
}

Command parseCommand(fy_node* node)
{
    Command command;
    if (fy_node_is_mapping(node)) {
        std::set<std::string> fields;
        void* iterator = nullptr;
        while (fy_node_pair* pair = fy_node_mapping_iterate(node, &iterator)) {
            const std::string field = scalar(fy_node_pair_key(pair), "command field");
            const std::string value = scalar(fy_node_pair_value(pair), field);
            if (!fields.insert(field).second) {
                throw std::runtime_error("Duplicate command field: " + field);
            }
            if (field == "command") {
                command.text = value;
            } else if (field == "background") {
                if (value != "true" && value != "false") {
                    throw std::runtime_error("background must be true or false");
                }
                command.background = value == "true";
            } else if (field == "wait_for_socket") {
                command.socketPath = value;
                if (value.empty() || value.size() >= sizeof(sockaddr_un::sun_path)) {
                    throw std::runtime_error("wait_for_socket must be a nonempty Unix socket path");
                }
            } else if (field == "timeout_seconds") {
                if (value.empty() || value.find_first_not_of("0123456789") != std::string::npos) {
                    throw std::runtime_error("timeout_seconds must be a positive integer");
                }
                const unsigned long timeout = std::stoul(value);
                if (timeout == 0 || timeout > 3600) {
                    throw std::runtime_error("timeout_seconds must be between 1 and 3600");
                }
                command.timeoutSeconds = static_cast<unsigned int>(timeout);
            } else {
                throw std::runtime_error("Unknown command field: " + field);
            }
        }
    } else {
        command.text = scalar(node, "command");
    }
    const size_t end = command.text.find_last_not_of(" \t\r\n");
    if (end != std::string::npos && command.text[end] == '&' &&
        (end == 0 || command.text[end - 1] != '&')) {
        command.background = true;
        command.text.erase(end);
    }
    if (command.text.find_first_not_of(" \t\r\n") == std::string::npos) {
        throw std::runtime_error("Environment commands cannot be empty");
    }
    if (!command.background && !command.socketPath.empty()) {
        throw std::runtime_error("wait_for_socket requires a background command");
    }
    return command;
}

struct SpawnConfiguration
{
    posix_spawn_file_actions_t actions;
    posix_spawnattr_t attributes;

    SpawnConfiguration()
    {
        int error = posix_spawn_file_actions_init(&actions);
        if (error != 0) {
            throw std::runtime_error(std::string("Could not initialize spawn actions: ") +
                std::strerror(error));
        }
        error = posix_spawnattr_init(&attributes);
        if (error != 0) {
            posix_spawn_file_actions_destroy(&actions);
            throw std::runtime_error(std::string("Could not initialize spawn attributes: ") +
                std::strerror(error));
        }
    }

    ~SpawnConfiguration()
    {
        posix_spawn_file_actions_destroy(&actions);
        posix_spawnattr_destroy(&attributes);
    }
};

void checkSpawnOperation(int error)
{
    if (error != 0) {
        throw std::runtime_error(std::string("Background process setup failed: ") +
            std::strerror(error));
    }
}

pid_t startBackground(const Command& command, std::string& outputPath)
{
    SpawnConfiguration setup;
    checkSpawnOperation(posix_spawnattr_setflags(&setup.attributes, POSIX_SPAWN_SETPGROUP));
    checkSpawnOperation(posix_spawnattr_setpgroup(&setup.attributes, 0));
    char logPath[] = "/tmp/mp-background-XXXXXX";
    const int log = mkstemp(logPath);
    if (log < 0) {
        throw std::runtime_error(std::string("Could not create background log: ") +
            std::strerror(errno));
    }
    if (close(log) != 0) {
        throw std::runtime_error(std::string("Could not close background log: ") +
            std::strerror(errno));
    }
    outputPath = logPath;
    checkSpawnOperation(posix_spawn_file_actions_addopen(
        &setup.actions, STDIN_FILENO, "/dev/null", O_RDONLY, 0));
    checkSpawnOperation(posix_spawn_file_actions_addopen(
        &setup.actions, STDOUT_FILENO, logPath, O_WRONLY | O_APPEND, 0600));
    checkSpawnOperation(posix_spawn_file_actions_adddup2(
        &setup.actions, STDOUT_FILENO, STDERR_FILENO));

    // The shell and its children stay in an owned process group for cleanup.
    char shell[] = "/bin/sh";
    char option[] = "-c";
    std::string text = command.text;
    char* arguments[] = {shell, option, text.data(), nullptr};
    pid_t pid = -1;
    const int error = posix_spawn(
        &pid, shell, &setup.actions, &setup.attributes, arguments, environ);
    if (error != 0) {
        throw std::runtime_error(std::string("Could not start background command: ") +
            std::strerror(error));
    }
    UT_LOG("[ENV] Background process PID=%ld; output: %s", static_cast<long>(pid), logPath);
    return pid;
}

void stopBackground(pid_t pid)
{
    // Do not reap the leader until after killing its group, preventing PID reuse.
    if (kill(-pid, SIGKILL) != 0 && errno != ESRCH) {
        UT_LOG("ERROR: Could not kill background process group %ld: %s",
            static_cast<long>(pid), std::strerror(errno));
    }
    int status = 0;
    pid_t result;
    do {
        result = waitpid(pid, &status, 0);
    } while (result < 0 && errno == EINTR);
    if (result < 0) {
        UT_LOG("ERROR: Could not reap background process %ld: %s\n",
            static_cast<long>(pid), std::strerror(errno));
    } else {
        UT_LOG("[ENV] Stopped background process PID=%ld\n", static_cast<long>(pid));
    }
}

bool socketReady(const std::string& path)
{
    const int descriptor = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (descriptor < 0) {
        throw std::runtime_error(std::string("Could not create readiness socket: ") +
            std::strerror(errno));
    }
    sockaddr_un address{};
    address.sun_family = AF_UNIX;
    std::memcpy(address.sun_path, path.c_str(), path.size() + 1);
    const int result = connect(descriptor, reinterpret_cast<sockaddr*>(&address), sizeof(address));
    const int error = errno;
    if (close(descriptor) != 0) {
        throw std::runtime_error(std::string("Could not close readiness socket: ") +
            std::strerror(errno));
    }
    if (result == 0) {
        return true;
    }
    if (error == ENOENT || error == ECONNREFUSED || error == EAGAIN ||
        error == EINPROGRESS || error == EINTR) {
        return false;
    }
    throw std::runtime_error("Could not connect to readiness socket " + path +
        ": " + std::strerror(error));
}

void printBackgroundLog(const std::string& path)
{
    UT_LOG("[ENV] Background command output: %s", path.c_str());
    std::ifstream file(path);
    if (!file) {
        UT_LOG("ERROR: Could not read background command log: %s", path.c_str());
        return;
    }
    std::deque<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
        if (lines.size() > 50) {
            lines.pop_front();
        }
    }
    if (file.bad()) {
        UT_LOG("ERROR: Failed while reading background command log");
    }
    for (const auto& entry : lines) {
        UT_LOG("[BACKGROUND] %s", entry.c_str());
    }
}

void waitForReady(pid_t pid, const Command& command, const std::string& outputPath)
{
    const auto deadline = std::chrono::steady_clock::now() +
        std::chrono::seconds(command.timeoutSeconds);
    do {
        siginfo_t information{};
        if (waitid(P_PID, pid, &information, WEXITED | WNOHANG | WNOWAIT) != 0) {
            if (errno == EINTR) {
                continue;
            }
            throw std::runtime_error(std::string("Could not check background process: ") +
                std::strerror(errno));
        }
        if (information.si_pid != 0) {
            printBackgroundLog(outputPath);
            throw std::runtime_error("Background command exited before readiness (status=" +
                std::to_string(information.si_status) + "): " + command.text);
        }
        if (!command.socketPath.empty() && socketReady(command.socketPath)) {
            UT_LOG("[ENV] Background process ready: %s", command.socketPath.c_str());
            return;
        }
        const auto now = std::chrono::steady_clock::now();
        if (now >= deadline) {
            if (command.socketPath.empty()) {
                UT_LOG("[ENV] Background PID=%ld still alive after %u seconds; command succeeded",
                    static_cast<long>(pid), command.timeoutSeconds);
                return;
            }
            break;
        }
        std::this_thread::sleep_until(std::min(deadline, now + std::chrono::milliseconds(100)));
    } while (true);
    printBackgroundLog(outputPath);
    throw std::runtime_error("Timed out waiting for socket: " + command.socketPath);
}

}

void EnvironmentSetup::initialize(const std::string& path)
{
    if (!m_savedVariables.empty() || !m_libraries.empty() || !m_backgroundProcesses.empty()) {
        throw std::runtime_error("Environment setup is already active");
    }

    UT_LOG("[ENV] Loading configuration: %s", path.c_str());
    std::unique_ptr<fy_document, decltype(&fy_document_destroy)> document(
        fy_document_build_from_file(nullptr, path.c_str()), fy_document_destroy);
    if (!document) {
        throw std::runtime_error("Could not read or parse environment configuration: " + path);
    }
    fy_node* root = fy_document_root(document.get());
    if (!root || !fy_node_is_mapping(root)) {
        throw std::runtime_error("Environment configuration must be a YAML mapping");
    }

    std::vector<std::pair<std::string, std::string>> variables;
    std::vector<Command> commands;
    std::set<std::string> sections;
    void* iterator = nullptr;
    while (fy_node_pair* pair = fy_node_mapping_iterate(root, &iterator)) {
        const std::string section = scalar(fy_node_pair_key(pair), "section");
        if (!sections.insert(section).second) {
            throw std::runtime_error("Duplicate environment configuration section: " + section);
        }
        fy_node* value = fy_node_pair_value(pair);
        if (section == "environment") {
            if (!value || !fy_node_is_mapping(value)) {
                throw std::runtime_error("environment must be a YAML mapping");
            }
            std::set<std::string> names;
            void* variableIterator = nullptr;
            while (fy_node_pair* variable = fy_node_mapping_iterate(value, &variableIterator)) {
                std::string name = scalar(fy_node_pair_key(variable), "variable name");
                if (name.empty() || name.find('=') != std::string::npos) {
                    throw std::runtime_error("Invalid environment variable name: " + name);
                }
                if (!names.insert(name).second) {
                    throw std::runtime_error("Duplicate environment variable: " + name);
                }
                variables.emplace_back(name, scalar(fy_node_pair_value(variable), name));
            }
        } else if (section == "commands") {
            if (!value || !fy_node_is_sequence(value)) {
                throw std::runtime_error("commands must be a YAML sequence");
            }
            void* commandIterator = nullptr;
            while (fy_node* command = fy_node_sequence_iterate(value, &commandIterator)) {
                commands.push_back(parseCommand(command));
            }
        } else {
            throw std::runtime_error("Unknown environment configuration section: " + section);
        }
    }

    std::vector<std::string> libraries;
    for (const auto& variable : variables) {
        if (variable.first == "LD_PRELOAD") {
            std::string preload = variable.second;
            for (char& character : preload) {
                if (character == ':') {
                    character = ' ';
                }
            }
            std::istringstream stream(preload);
            std::string library;
            while (stream >> library) {
                libraries.push_back(library);
            }
        }
    }

    for (const auto& variable : variables) {
        const char* previous = std::getenv(variable.first.c_str());
        m_savedVariables.push_back({variable.first,
            previous ? std::optional<std::string>(previous) : std::nullopt});
        if (setenv(variable.first.c_str(), variable.second.c_str(), 1) != 0) {
            throw std::runtime_error("Could not set " + variable.first + ": " + std::strerror(errno));
        }
        UT_LOG("[ENV] Set %s=%s", variable.first.c_str(), variable.second.c_str());
    }

    for (const std::string& library : libraries) {
        dlerror();
        void* handle = dlopen(library.c_str(), RTLD_NOW | RTLD_GLOBAL);
        if (!handle) {
            const char* error = dlerror();
            throw std::runtime_error("Could not load " + library + ": " +
                (error ? error : "unknown dlopen error"));
        }
        try {
            m_libraries.push_back(handle);
        } catch (const std::bad_alloc&) {
            if (dlclose(handle) != 0) {
                UT_LOG("ERROR: Could not close library: %s", dlerror());
            }
            throw;
        }
        UT_LOG("[ENV] Loaded library: %s", library.c_str());
    }

    for (const Command& command : commands) {
        UT_LOG("[ENV] Executing command: %s", command.text.c_str());
        if (command.background) {
            if (!command.socketPath.empty() && socketReady(command.socketPath)) {
                throw std::runtime_error("Readiness socket already in use: " + command.socketPath);
            }
            std::string outputPath;
            const pid_t pid = startBackground(command, outputPath);
            try {
                m_backgroundProcesses.push_back(pid);
            } catch (const std::bad_alloc&) {
                stopBackground(pid);
                throw;
            }
            waitForReady(pid, command, outputPath);
            continue;
        }
        const int status = std::system(command.text.c_str());
        if (status == -1) {
            throw std::runtime_error("Could not execute command: " + command.text +
                ": " + std::strerror(errno));
        }
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            throw std::runtime_error("Environment command failed (status=" +
                std::to_string(status) + "): " + command.text);
        }
    }
}

EnvironmentSetup::~EnvironmentSetup()
{
    for (auto process = m_backgroundProcesses.rbegin();
         process != m_backgroundProcesses.rend(); ++process) {
        stopBackground(*process);
    }
    for (auto handle = m_libraries.rbegin(); handle != m_libraries.rend(); ++handle) {
        if (dlclose(*handle) != 0) {
            UT_LOG("ERROR: Could not close environment library: %s", dlerror());
        }
    }
    for (auto variable = m_savedVariables.rbegin(); variable != m_savedVariables.rend(); ++variable) {
        const int result = variable->value ?
            setenv(variable->name.c_str(), variable->value->c_str(), 1) :
            unsetenv(variable->name.c_str());
        if (result != 0) {
            UT_LOG("ERROR: Could not restore environment variable %s: %s",
                variable->name.c_str(), std::strerror(errno));
        }
    }
}
