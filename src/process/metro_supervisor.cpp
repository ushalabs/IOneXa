#include "metro_supervisor.hpp"
#include "core/structured_logger.hpp"

#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QDir>
#include <thread>
#include <chrono>

namespace ionexa::process {

MetroSupervisor::MetroSupervisor(QObject* parent)
    : QObject(parent)
    , m_process(std::make_unique<ProcessSupervisor>()) {}

MetroSupervisor::~MetroSupervisor() {
    stop();
}

int MetroSupervisor::findAvailablePort(int startPort, int maxAttempts) {
    for (int port = startPort; port < startPort + maxAttempts; ++port) {
        QTcpServer server;
        if (server.listen(QHostAddress::LocalHost, static_cast<quint16>(port))) {
            server.close();
            return port;
        }
    }
    return startPort;
}

bool MetroSupervisor::start(const project::ProjectDetails& project, int preferredPort) {
    if (m_status == MetroStatus::Starting || m_status == MetroStatus::Ready) {
        return false;
    }

    m_currentProject = project;
    m_activePort = findAvailablePort(preferredPort);
    m_stopRequested.store(false);

    m_status = MetroStatus::Starting;
    emit statusChanged(m_status);

    core::StructuredLogger::instance().info(
        core::LogSource::Metro,
        "Launching Metro dev server on port " + std::to_string(m_activePort),
        "dir=" + project.canonicalPath
    );

    // Build process invocation
    // Use cmd.exe /c npx expo start --web --port <port> on Windows
    std::wstring appPath = L"cmd.exe";
    std::wstring args = L"/c npx.cmd expo start --web --port " + std::to_wstring(m_activePort);
    std::wstring workingDir = QString::fromStdString(project.canonicalPath).toStdWString();

    bool launched = m_process->start(
        appPath,
        args,
        workingDir,
        [this](const std::string& line, bool isStderr) {
            core::StructuredLogger::instance().log(
                isStderr ? core::LogLevel::Warning : core::LogLevel::Info,
                core::LogSource::Metro,
                line
            );
            QMetaObject::invokeMethod(this, [this, qline = QString::fromStdString(line), isStderr]() {
                emit logOutput(qline, isStderr);
            });
        },
        [this](int exitCode) {
            core::StructuredLogger::instance().info(
                core::LogSource::Metro,
                "Metro process exited with code " + std::to_string(exitCode)
            );
            QMetaObject::invokeMethod(this, [this, exitCode]() {
                m_status = MetroStatus::Stopped;
                emit statusChanged(m_status);
                emit processTerminated(exitCode);
            });
        }
    );

    if (!launched) {
        m_status = MetroStatus::Error;
        emit statusChanged(m_status);
        emit errorOccurred("Failed to spawn Metro supervisor process");
        return false;
    }

    // Monitor readiness in background thread
    std::thread([this, port = m_activePort]() {
        monitorReadiness(port);
    }).detach();

    return true;
}

void MetroSupervisor::monitorReadiness(int port, int timeoutMs) {
    const auto startTime = std::chrono::steady_clock::now();
    bool isReady = false;

    while (!m_stopRequested.load()) {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - startTime).count();

        if (elapsed > timeoutMs) {
            break;
        }

        // Test TCP connection to loopback port
        QTcpSocket socket;
        socket.connectToHost(QHostAddress::LocalHost, static_cast<quint16>(port));
        if (socket.waitForConnected(500)) {
            socket.disconnectFromHost();
            isReady = true;
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    if (isReady && !m_stopRequested.load()) {
        QMetaObject::invokeMethod(this, [this, port]() {
            m_status = MetroStatus::Ready;
            emit statusChanged(m_status);
            emit ready(loopbackUrl(), port);
        });
    }
}

bool MetroSupervisor::stop(int timeoutMs) {
    m_stopRequested.store(true);
    if (m_status == MetroStatus::Idle || m_status == MetroStatus::Stopped) {
        return true;
    }

    m_status = MetroStatus::Stopping;
    emit statusChanged(m_status);

    bool res = m_process->stop(timeoutMs);

    m_status = MetroStatus::Stopped;
    emit statusChanged(m_status);
    return res;
}

bool MetroSupervisor::restart(int timeoutMs) {
    if (!stop(timeoutMs)) {
        return false;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    return start(m_currentProject, m_activePort);
}

MetroStatus MetroSupervisor::status() const {
    return m_status;
}

int MetroSupervisor::activePort() const {
    return m_activePort;
}

int MetroSupervisor::processId() const {
    return m_process ? m_process->processId() : 0;
}

QString MetroSupervisor::loopbackUrl() const {
    return QString("http://127.0.0.1:%1").arg(m_activePort);
}

} // namespace ionexa::process
