#include "fast_refresh_bridge.hpp"
#include "core/structured_logger.hpp"

#include <QRegularExpression>
#include <QJsonDocument>
#include <QJsonObject>

namespace ionexa::engines {

FastRefreshBridge::FastRefreshBridge(QObject* parent)
    : QObject(parent),
      m_healthTimer(new QTimer(this)),
      m_reconnectTimer(new QTimer(this)) {

    m_healthTimer->setInterval(3000);
    connect(m_healthTimer, &QTimer::timeout, this, &FastRefreshBridge::onHealthTimerTick);

    m_reconnectTimer->setInterval(1500);
    connect(m_reconnectTimer, &QTimer::timeout, this, &FastRefreshBridge::onReconnectTimerTick);
}

FastRefreshBridge::~FastRefreshBridge() {
    m_healthTimer->stop();
    m_reconnectTimer->stop();
}

void FastRefreshBridge::setMetroEndpoint(const QString& host, int port) {
    if (m_port != 0 && m_port != port && m_state == BridgeState::Connected) {
        emit staleSessionDetected(QString("Metro port changed unexpectedly from %1 to %2").arg(m_port).arg(port));
    }
    m_host = host.isEmpty() ? "127.0.0.1" : host;
    m_port = port;
}

void FastRefreshBridge::setState(BridgeState state) {
    if (m_state != state) {
        BridgeState oldState = m_state;
        m_state = state;
        emit stateChanged(state, oldState);
    }
}

void FastRefreshBridge::markDevServerStarted(int port) {
    m_port = port;
    m_consecutiveHealthFailures = 0;
    m_currentReconnectAttempt = 0;
    m_reconnectTimer->stop();

    setState(BridgeState::Connecting);
    if (checkHealthSync(1000)) {
        setState(BridgeState::Connected);
        m_healthTimer->start();
    } else {
        // Dev server process started, wait for socket readiness
        m_healthTimer->start();
    }
}

void FastRefreshBridge::markDevServerStopped() {
    m_healthTimer->stop();
    m_reconnectTimer->stop();
    m_consecutiveHealthFailures = 0;
    m_currentReconnectAttempt = 0;
    setState(BridgeState::Idle);
}

void FastRefreshBridge::reset() {
    markDevServerStopped();
    m_stats = FastRefreshStats{};
    m_port = 0;
    emit statsUpdated(m_stats);
}

bool FastRefreshBridge::checkHealthSync(int timeoutMs) {
    if (m_port <= 0) {
        return false;
    }

    QTcpSocket socket;
    socket.connectToHost(m_host, static_cast<quint16>(m_port));
    bool connected = socket.waitForConnected(timeoutMs);
    if (connected) {
        socket.disconnectFromHost();
    }
    return connected;
}

void FastRefreshBridge::performHealthCheck() {
    bool healthy = checkHealthSync(800);

    if (healthy) {
        m_consecutiveHealthFailures = 0;
        if (m_state != BridgeState::Connected) {
            setState(BridgeState::Connected);
            emit reconnected();
            emit reloadRequested(false);
        }
    } else {
        m_consecutiveHealthFailures++;
        if (m_consecutiveHealthFailures >= 2) {
            if (m_state == BridgeState::Connected) {
                setState(BridgeState::Reconnecting);
                emit staleSessionDetected("Metro dev server lost loopback connection");

                if (m_autoReconnect) {
                    m_healthTimer->stop();
                    m_currentReconnectAttempt = 0;
                    m_reconnectTimer->start();
                } else {
                    setState(BridgeState::Stale);
                }
            }
        }
    }
}

void FastRefreshBridge::onHealthTimerTick() {
    performHealthCheck();
}

void FastRefreshBridge::onReconnectTimerTick() {
    m_currentReconnectAttempt++;
    m_stats.reconnectAttempts++;
    emit reconnectAttempt(m_currentReconnectAttempt, m_maxReconnectAttempts);
    emit statsUpdated(m_stats);

    if (checkHealthSync(1000)) {
        m_reconnectTimer->stop();
        m_consecutiveHealthFailures = 0;
        m_currentReconnectAttempt = 0;
        setState(BridgeState::Connected);
        m_healthTimer->start();

        core::StructuredLogger::instance().info(
            core::LogSource::Engine,
            QString("Metro dev server reconnected on port %1").arg(m_port).toStdString()
        );

        emit reconnected();
        emit reloadRequested(false);
    } else if (m_currentReconnectAttempt >= m_maxReconnectAttempts) {
        m_reconnectTimer->stop();
        setState(BridgeState::Stale);
        emit staleSessionDetected(QString("Exceeded %1 reconnect attempts without response").arg(m_maxReconnectAttempts));
    }
}

void FastRefreshBridge::requestManualReload(bool hard) {
    m_stats.totalReloads++;
    emit reloadRequested(hard);
    emit statsUpdated(m_stats);
}

void FastRefreshBridge::processMetroLog(const QString& line) {
    static const QRegularExpression bundleDoneRegex(R"(Bundled\s+.*?(\d+)ms)", QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression bundlingRegex(R"(Bundling\s+|Building JavaScript bundle)", QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression fastRefreshRegex(R"(Fast Refresh|\[Fast Refresh\])", QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression errorRegex(R"(Failed to compile|SyntaxError:|TransformError:|Unable to resolve module)", QRegularExpression::CaseInsensitiveOption);

    if (errorRegex.match(line).hasMatch()) {
        m_stats.bundleErrors++;
        m_stats.lastErrorTime = QDateTime::currentDateTime();
        m_stats.lastErrorMessage = line.trimmed();
        emit bundleError(line.trimmed());
        emit statsUpdated(m_stats);
        return;
    }

    if (fastRefreshRegex.match(line).hasMatch()) {
        m_stats.fastRefreshCount++;
        m_stats.lastRefreshTime = QDateTime::currentDateTime();
        emit fastRefreshDetected(line.trimmed());
        emit statsUpdated(m_stats);
        return;
    }

    auto match = bundleDoneRegex.match(line);
    if (match.hasMatch()) {
        int durationMs = match.captured(1).toInt();
        m_stats.fastRefreshCount++;
        m_stats.lastRefreshTime = QDateTime::currentDateTime();
        emit bundleCompleted(durationMs);
        emit statsUpdated(m_stats);
        return;
    }

    if (bundlingRegex.match(line).hasMatch()) {
        emit bundleStarted();
    }
}

void FastRefreshBridge::processWebMessage(const QString& jsonString) {
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());
    if (!doc.isObject()) {
        return;
    }

    QJsonObject obj = doc.object();
    QString type = obj.value("type").toString();

    if (type == "fast_refresh_applied") {
        m_stats.fastRefreshCount++;
        m_stats.lastRefreshTime = QDateTime::currentDateTime();
        emit fastRefreshDetected("WebView2 client confirmed Fast Refresh applied");
        emit statsUpdated(m_stats);
    } else if (type == "runtime_error") {
        m_stats.bundleErrors++;
        m_stats.lastErrorTime = QDateTime::currentDateTime();
        m_stats.lastErrorMessage = obj.value("message").toString();
        emit bundleError(m_stats.lastErrorMessage);
        emit statsUpdated(m_stats);
    } else if (type == "page_loaded") {
        if (m_state != BridgeState::Connected && m_port > 0) {
            setState(BridgeState::Connected);
        }
    }
}

} // namespace ionexa::engines
