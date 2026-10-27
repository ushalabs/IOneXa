#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace ionexa::process {

using LogCallback = std::function<void(const std::string& line, bool isStderr)>;
using ExitCallback = std::function<void(int exitCode)>;

class ProcessSupervisor {
public:
    ProcessSupervisor();
    ~ProcessSupervisor();

    ProcessSupervisor(const ProcessSupervisor&) = delete;
    ProcessSupervisor& operator=(const ProcessSupervisor&) = delete;

    bool start(const std::wstring& applicationPath,
               const std::wstring& arguments,
               const std::wstring& workingDirectory,
               LogCallback onLog = nullptr,
               ExitCallback onExit = nullptr);

    bool stop(int timeoutMs = 3000);
    [[nodiscard]] bool isRunning() const;
    [[nodiscard]] int processId() const;

private:
    void cleanup();

#ifdef _WIN32
    HANDLE m_jobHandle{nullptr};
    HANDLE m_processHandle{nullptr};
    HANDLE m_threadHandle{nullptr};
    HANDLE m_stdoutRead{nullptr};
    HANDLE m_stderrRead{nullptr};
    DWORD m_pid{0};
#else
    int m_pid{0};
#endif
    bool m_running{false};
};

} // namespace ionexa::process
