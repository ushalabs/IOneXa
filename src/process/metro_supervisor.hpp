#pragma once

#include <QObject>
#include <QString>
#include <memory>
#include <functional>
#include <atomic>
#include <thread>

#include "process_supervisor.hpp"
#include "project/project_scanner.hpp"

namespace ionexa::process {

enum class MetroStatus {
    Idle,
    Starting,
    Ready,
    Stopping,
    Stopped,
    Error
};

class MetroSupervisor : public QObject {
    Q_OBJECT

public:
    explicit MetroSupervisor(QObject* parent = nullptr);
    ~MetroSupervisor() override;

    bool start(const project::ProjectDetails& project, int preferredPort = 8081);
    bool stop(int timeoutMs = 4000);
    bool restart(int timeoutMs = 4000);

    [[nodiscard]] MetroStatus status() const;
    [[nodiscard]] int activePort() const;
    [[nodiscard]] int processId() const;
    [[nodiscard]] QString loopbackUrl() const;

    static int findAvailablePort(int startPort = 8081, int maxAttempts = 50);

signals:
    void statusChanged(MetroStatus newStatus);
    void ready(const QString& url, int port);
    void logOutput(const QString& line, bool isStderr);
    void processTerminated(int exitCode);
    void errorOccurred(const QString& errorMessage);

private:
    void monitorReadiness(int port, int timeoutMs = 45000);

    std::unique_ptr<ProcessSupervisor> m_process;
    project::ProjectDetails m_currentProject;
    MetroStatus m_status{MetroStatus::Idle};
    int m_activePort{0};
    std::atomic<bool> m_stopRequested{false};
    std::shared_ptr<std::atomic<bool>> m_alive{std::make_shared<std::atomic<bool>>(true)};
    std::thread m_monitorThread;
};

} // namespace ionexa::process
