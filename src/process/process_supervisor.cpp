#include "process_supervisor.hpp"
#include <iostream>
#include <thread>
#include <vector>

namespace ionexa::process {

ProcessSupervisor::ProcessSupervisor() {
#ifdef _WIN32
    // Create Windows Job Object with KILL_ON_JOB_CLOSE limit to prevent orphaned processes
    m_jobHandle = CreateJobObjectW(nullptr, nullptr);
    if (m_jobHandle) {
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = {};
        jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        SetInformationJobObject(m_jobHandle, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli));
    }
#endif
}

ProcessSupervisor::~ProcessSupervisor() {
    stop();
    cleanup();
}

void ProcessSupervisor::cleanup() {
#ifdef _WIN32
    if (m_stdoutRead) {
        CloseHandle(m_stdoutRead);
        m_stdoutRead = nullptr;
    }
    if (m_stderrRead) {
        CloseHandle(m_stderrRead);
        m_stderrRead = nullptr;
    }
    if (m_threadHandle) {
        CloseHandle(m_threadHandle);
        m_threadHandle = nullptr;
    }
    if (m_processHandle) {
        CloseHandle(m_processHandle);
        m_processHandle = nullptr;
    }
    if (m_jobHandle) {
        CloseHandle(m_jobHandle);
        m_jobHandle = nullptr;
    }
#endif
    m_running = false;
    m_pid = 0;
}

bool ProcessSupervisor::start(const std::wstring& applicationPath,
                              const std::wstring& arguments,
                              const std::wstring& workingDirectory,
                              LogCallback onLog,
                              ExitCallback onExit) {
    if (m_running) {
        return false;
    }

#ifdef _WIN32
    SECURITY_ATTRIBUTES sa = {};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;

    HANDLE stdoutWrite = nullptr;
    HANDLE stderrWrite = nullptr;

    if (!CreatePipe(&m_stdoutRead, &stdoutWrite, &sa, 0)) return false;
    SetHandleInformation(m_stdoutRead, HANDLE_FLAG_INHERIT, 0);

    if (!CreatePipe(&m_stderrRead, &stderrWrite, &sa, 0)) {
        CloseHandle(m_stdoutRead);
        CloseHandle(stdoutWrite);
        return false;
    }
    SetHandleInformation(m_stderrRead, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOW si = {};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    si.hStdOutput = stdoutWrite;
    si.hStdError = stderrWrite;
    si.hStdInput = nullptr;

    PROCESS_INFORMATION pi = {};

    std::wstring cmdLine = arguments.empty() ? applicationPath : (applicationPath + L" " + arguments);
    std::vector<wchar_t> cmdBuffer(cmdLine.begin(), cmdLine.end());
    cmdBuffer.push_back(L'\0');

    BOOL success = CreateProcessW(
        nullptr,
        cmdBuffer.data(),
        nullptr,
        nullptr,
        TRUE,
        CREATE_SUSPENDED | CREATE_NO_WINDOW,
        nullptr,
        workingDirectory.empty() ? nullptr : workingDirectory.c_str(),
        &si,
        &pi
    );

    CloseHandle(stdoutWrite);
    CloseHandle(stderrWrite);

    if (!success) {
        CloseHandle(m_stdoutRead);
        CloseHandle(m_stderrRead);
        m_stdoutRead = nullptr;
        m_stderrRead = nullptr;
        return false;
    }

    m_processHandle = pi.hProcess;
    m_threadHandle = pi.hThread;
    m_pid = pi.dwProcessId;
    m_running = true;

    if (m_jobHandle) {
        AssignProcessToJobObject(m_jobHandle, m_processHandle);
    }

    // Resume process execution
    ResumeThread(m_threadHandle);

    // Read stdout in background thread
    if (onLog && m_stdoutRead) {
        HANDLE hOut = m_stdoutRead;
        std::thread([hOut, onLog]() {
            char buffer[1024];
            DWORD bytesRead = 0;
            std::string line;
            while (ReadFile(hOut, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) && bytesRead > 0) {
                buffer[bytesRead] = '\0';
                for (DWORD i = 0; i < bytesRead; ++i) {
                    if (buffer[i] == '\n') {
                        if (!line.empty() && line.back() == '\r') line.pop_back();
                        onLog(line, false);
                        line.clear();
                    } else {
                        line += buffer[i];
                    }
                }
            }
            if (!line.empty()) {
                onLog(line, false);
            }
        }).detach();
    }

    // Wait for exit in background thread
    if (onExit && m_processHandle) {
        HANDLE hProc = m_processHandle;
        std::thread([this, hProc, onExit]() {
            WaitForSingleObject(hProc, INFINITE);
            DWORD exitCode = 0;
            GetExitCodeProcess(hProc, &exitCode);
            m_running = false;
            onExit(static_cast<int>(exitCode));
        }).detach();
    }

    return true;
#else
    return false;
#endif
}

bool ProcessSupervisor::stop(int timeoutMs) {
    if (!m_running) {
        return true;
    }

#ifdef _WIN32
    if (m_processHandle) {
        // If assigned to job object with KILL_ON_JOB_CLOSE, terminating the job kills all children
        if (m_jobHandle) {
            TerminateJobObject(m_jobHandle, 0);
        } else {
            TerminateProcess(m_processHandle, 0);
        }

        WaitForSingleObject(m_processHandle, timeoutMs);
    }
#endif
    m_running = false;
    return true;
}

bool ProcessSupervisor::isRunning() const {
    return m_running;
}

int ProcessSupervisor::processId() const {
    return static_cast<int>(m_pid);
}

} // namespace ionexa::process
