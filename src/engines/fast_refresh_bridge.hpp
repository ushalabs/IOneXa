#pragma once

#include <QObject>
#include <QString>
#include <QTimer>
#include <QDateTime>
#include <QTcpSocket>

namespace ionexa::engines {

enum class BridgeState {
    Idle,
    Connecting,
    Connected,
    Reconnecting,
    Stale
};

struct FastRefreshStats {
    int totalReloads{0};
    int fastRefreshCount{0};
    int bundleErrors{0};
    int reconnectAttempts{0};
    QDateTime lastRefreshTime;
    QDateTime lastErrorTime;
    QString lastErrorMessage;
};

class FastRefreshBridge : public QObject {
    Q_OBJECT

public:
    explicit FastRefreshBridge(QObject* parent = nullptr);
    ~FastRefreshBridge() override;

    void setMetroEndpoint(const QString& host, int port);
    [[nodiscard]] QString host() const { return m_host; }
    [[nodiscard]] int port() const { return m_port; }
    [[nodiscard]] BridgeState state() const { return m_state; }
    [[nodiscard]] FastRefreshStats stats() const { return m_stats; }
    [[nodiscard]] bool isAutoReconnectEnabled() const { return m_autoReconnect; }

    void setAutoReconnect(bool enable) { m_autoReconnect = enable; }
    void setStaleThresholdMs(int ms) { m_staleThresholdMs = ms; }
    void setMaxReconnectAttempts(int maxAttempts) { m_maxReconnectAttempts = maxAttempts; }

    // Metro process log inspector
    void processMetroLog(const QString& line);

    // WebView2 IPC message inspector
    void processWebMessage(const QString& jsonString);

    // Manual controls
    void requestManualReload(bool hard = false);
    bool checkHealthSync(int timeoutMs = 500);

    // Lifecycle hooks
    void markDevServerStarted(int port);
    void markDevServerStopped();
    void reset();

signals:
    void stateChanged(BridgeState newState, BridgeState oldState);
    void reloadRequested(bool hard);
    void fastRefreshDetected(const QString& details);
    void bundleStarted();
    void bundleCompleted(int durationMs);
    void bundleError(const QString& errorMessage);
    void staleSessionDetected(const QString& reason);
    void reconnectAttempt(int attempt, int maxAttempts);
    void reconnected();
    void statsUpdated(const FastRefreshStats& stats);

private slots:
    void onHealthTimerTick();
    void onReconnectTimerTick();

private:
    void setState(BridgeState state);
    void performHealthCheck();

    QString m_host{"127.0.0.1"};
    int m_port{0};
    BridgeState m_state{BridgeState::Idle};
    FastRefreshStats m_stats{};
    bool m_autoReconnect{true};
    int m_staleThresholdMs{10000};
    int m_maxReconnectAttempts{15};
    int m_consecutiveHealthFailures{0};
    int m_currentReconnectAttempt{0};

    QTimer* m_healthTimer{nullptr};
    QTimer* m_reconnectTimer{nullptr};
};

} // namespace ionexa::engines
