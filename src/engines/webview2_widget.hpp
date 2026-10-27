#pragma once

#include <QWidget>
#include <QString>
#include <memory>
#include <functional>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wrl.h>
#include "WebView2.h"
#endif

namespace ionexa::engines {

class WebView2Widget : public QWidget {
    Q_OBJECT

public:
    explicit WebView2Widget(QWidget* parent = nullptr);
    ~WebView2Widget() override;

    void navigate(const QString& url);
    void reload(bool ignoreCache = false);
    void openDevTools();
    void setZoomFactor(double factor);
    void executeScript(const QString& script);
    void postWebMessageAsJson(const QString& json);
    void addScriptToExecuteOnDocumentCreated(const QString& script);
    void applyDeviceEmulation(int width, int height, double scaleFactor, bool touchEnabled, const QString& userAgent = "");

    [[nodiscard]] bool isInitialized() const;
    [[nodiscard]] QString currentUrl() const;

signals:
    void initialized();
    void navigationStarting(const QString& url);
    void navigationCompleted(bool success, int errorCode);
    void rendererCrashed(int reason);
    void consoleMessageReceived(const QString& message, int level);
    void webMessageReceived(const QString& message);
    void resourceRequested(const QString& url, const QString& method);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    void initWebView();
    void updateBounds();

#ifdef _WIN32
    Microsoft::WRL::ComPtr<ICoreWebView2Controller> m_controller;
    Microsoft::WRL::ComPtr<ICoreWebView2> m_webview;
    EventRegistrationToken m_navStartingToken{};
    EventRegistrationToken m_navCompletedToken{};
    EventRegistrationToken m_processFailedToken{};
    EventRegistrationToken m_webMessageReceivedToken{};
    EventRegistrationToken m_resourceRequestedToken{};
#endif

    bool m_initialized{false};
    QString m_pendingUrl;
    QString m_currentUrl;
};

} // namespace ionexa::engines
