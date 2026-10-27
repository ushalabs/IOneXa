#include "webview2_widget.hpp"
#include "core/structured_logger.hpp"

#include <QResizeEvent>
#include <QShowEvent>
#include <QDir>
#include <iostream>

#ifdef _WIN32
using namespace Microsoft::WRL;
#endif

namespace ionexa::engines {

WebView2Widget::WebView2Widget(QWidget* parent)
    : QWidget(parent) {
    setAttribute(Qt::WA_NativeWindow, true);
    setAttribute(Qt::WA_DontCreateNativeAncestors, true);
    setUpdatesEnabled(true);
}

WebView2Widget::~WebView2Widget() {
#ifdef _WIN32
    if (m_webview) {
        if (m_navStartingToken.value) m_webview->remove_NavigationStarting(m_navStartingToken);
        if (m_navCompletedToken.value) m_webview->remove_NavigationCompleted(m_navCompletedToken);
        if (m_processFailedToken.value) m_webview->remove_ProcessFailed(m_processFailedToken);
        if (m_webMessageReceivedToken.value) m_webview->remove_WebMessageReceived(m_webMessageReceivedToken);
    }
    if (m_controller) {
        m_controller->Close();
        m_controller.Reset();
    }
    m_webview.Reset();
#endif
}

void WebView2Widget::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    if (!m_initialized) {
        initWebView();
    }
}

void WebView2Widget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    updateBounds();
}

void WebView2Widget::updateBounds() {
#ifdef _WIN32
    if (m_controller) {
        RECT bounds;
        bounds.left = 0;
        bounds.top = 0;
        bounds.right = width();
        bounds.bottom = height();
        m_controller->put_Bounds(bounds);
    }
#endif
}

void WebView2Widget::initWebView() {
#ifdef _WIN32
    HWND hWnd = reinterpret_cast<HWND>(winId());
    if (!hWnd) {
        return;
    }

    wchar_t tempPath[MAX_PATH];
    GetTempPathW(MAX_PATH, tempPath);
    std::wstring userDataFolder = std::wstring(tempPath) + L"IOneXa_WebView2_UserData";
    CreateDirectoryW(userDataFolder.c_str(), nullptr);

    HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(
        nullptr, userDataFolder.c_str(), nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [this, hWnd](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
                if (FAILED(result) || !env) {
                    core::StructuredLogger::instance().error(
                        core::LogSource::Engine,
                        "Failed to create WebView2 Environment: 0x" + std::to_string(result)
                    );
                    return result;
                }

                env->CreateCoreWebView2Controller(
                    hWnd,
                    Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [this](HRESULT ctrlResult, ICoreWebView2Controller* controller) -> HRESULT {
                            if (FAILED(ctrlResult) || !controller) {
                                core::StructuredLogger::instance().error(
                                    core::LogSource::Engine,
                                    "Failed to create WebView2 Controller: 0x" + std::to_string(ctrlResult)
                                );
                                return ctrlResult;
                            }

                            m_controller = controller;
                            m_controller->get_CoreWebView2(&m_webview);

                            updateBounds();
                            m_controller->put_IsVisible(TRUE);

                            // Listen for navigation starting
                            m_webview->add_NavigationStarting(
                                Callback<ICoreWebView2NavigationStartingEventHandler>(
                                    [this](ICoreWebView2*, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT {
                                        LPWSTR uri = nullptr;
                                        args->get_Uri(&uri);
                                        QString qUri = QString::fromWCharArray(uri);
                                        CoTaskMemFree(uri);

                                        emit navigationStarting(qUri);
                                        return S_OK;
                                    }).Get(),
                                &m_navStartingToken
                            );

                            // Listen for navigation completed
                            m_webview->add_NavigationCompleted(
                                Callback<ICoreWebView2NavigationCompletedEventHandler>(
                                    [this](ICoreWebView2*, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
                                        BOOL success = FALSE;
                                        args->get_IsSuccess(&success);
                                        COREWEBVIEW2_WEB_ERROR_STATUS status;
                                        args->get_WebErrorStatus(&status);

                                        m_currentUrl = m_pendingUrl;
                                        emit navigationCompleted(success == TRUE, static_cast<int>(status));
                                        return S_OK;
                                    }).Get(),
                                &m_navCompletedToken
                            );

                            // Listen for renderer process failures (crashes)
                            m_webview->add_ProcessFailed(
                                Callback<ICoreWebView2ProcessFailedEventHandler>(
                                    [this](ICoreWebView2*, ICoreWebView2ProcessFailedEventArgs* args) -> HRESULT {
                                        COREWEBVIEW2_PROCESS_FAILED_KIND kind;
                                        args->get_ProcessFailedKind(&kind);
                                        core::StructuredLogger::instance().error(
                                            core::LogSource::Engine,
                                            "WebView2 renderer process crashed (kind=" + std::to_string(kind) + ")"
                                        );
                                        emit rendererCrashed(static_cast<int>(kind));
                                        return S_OK;
                                    }).Get(),
                                &m_processFailedToken
                            );

                            // Listen for WebMessages (page IPC)
                            m_webview->add_WebMessageReceived(
                                Callback<ICoreWebView2WebMessageReceivedEventHandler>(
                                    [this](ICoreWebView2*, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
                                        LPWSTR messageRaw = nullptr;
                                        if (SUCCEEDED(args->TryGetWebMessageAsString(&messageRaw)) && messageRaw) {
                                            QString qMsg = QString::fromWCharArray(messageRaw);
                                            CoTaskMemFree(messageRaw);
                                            emit webMessageReceived(qMsg);
                                        }
                                        return S_OK;
                                    }).Get(),
                                &m_webMessageReceivedToken
                            );

                            // Inject IOneXa client runtime bridge script
                            const wchar_t* bridgeScript = LR"(
(function() {
    if (window.__ionexa_bridge_installed) return;
    window.__ionexa_bridge_installed = true;
    
    if (window.chrome && window.chrome.webview) {
        window.chrome.webview.postMessage(JSON.stringify({
            type: 'page_loaded',
            url: window.location.href,
            timestamp: Date.now()
        }));
    }

    window.addEventListener('message', function(ev) {
        try {
            if (ev.data && typeof ev.data === 'string') {
                var d = JSON.parse(ev.data);
                if (d.type === 'fast-refresh' || d.type === 'update-done') {
                    if (window.chrome && window.chrome.webview) {
                        window.chrome.webview.postMessage(JSON.stringify({
                            type: 'fast_refresh_applied',
                            timestamp: Date.now()
                        }));
                    }
                }
            }
        } catch(e) {}
    });

    window.addEventListener('error', function(err) {
        if (window.chrome && window.chrome.webview) {
            window.chrome.webview.postMessage(JSON.stringify({
                type: 'runtime_error',
                message: err.message || 'Script error',
                filename: err.filename || '',
                lineno: err.lineno || 0
            }));
        }
    });
})();
)";
                            m_webview->AddScriptToExecuteOnDocumentCreated(
                                bridgeScript,
                                Callback<ICoreWebView2AddScriptToExecuteOnDocumentCreatedCompletedHandler>(
                                    [](HRESULT, LPCWSTR) -> HRESULT { return S_OK; }
                                ).Get()
                            );

                            m_initialized = true;
                            emit initialized();

                            if (!m_pendingUrl.isEmpty()) {
                                navigate(m_pendingUrl);
                            }

                            return S_OK;
                        }).Get()
                );
                return S_OK;
            }).Get()
    );

    if (FAILED(hr)) {
        core::StructuredLogger::instance().error(
            core::LogSource::Engine,
            "CreateCoreWebView2EnvironmentWithOptions call failed: 0x" + std::to_string(hr)
        );
    }
#endif
}

void WebView2Widget::navigate(const QString& url) {
    m_pendingUrl = url;
#ifdef _WIN32
    if (m_webview) {
        std::wstring wUrl = url.toStdWString();
        m_webview->Navigate(wUrl.c_str());
    }
#endif
}

void WebView2Widget::reload(bool ignoreCache) {
#ifdef _WIN32
    if (m_webview) {
        if (ignoreCache) {
            m_webview->CallDevToolsProtocolMethod(
                L"Page.reload",
                L"{\"ignoreCache\":true}",
                nullptr
            );
        } else {
            m_webview->Reload();
        }
    }
#endif
}

void WebView2Widget::openDevTools() {
#ifdef _WIN32
    if (m_webview) {
        m_webview->OpenDevToolsWindow();
    }
#endif
}

void WebView2Widget::setZoomFactor(double factor) {
#ifdef _WIN32
    if (m_controller) {
        m_controller->put_ZoomFactor(factor);
    }
#endif
}

void WebView2Widget::executeScript(const QString& script) {
#ifdef _WIN32
    if (m_webview) {
        std::wstring wScript = script.toStdWString();
        m_webview->ExecuteScript(wScript.c_str(), nullptr);
    }
#endif
}

void WebView2Widget::postWebMessageAsJson(const QString& json) {
#ifdef _WIN32
    if (m_webview) {
        std::wstring wJson = json.toStdWString();
        m_webview->PostWebMessageAsJson(wJson.c_str());
    }
#endif
}

void WebView2Widget::addScriptToExecuteOnDocumentCreated(const QString& script) {
#ifdef _WIN32
    if (m_webview) {
        std::wstring wScript = script.toStdWString();
        m_webview->AddScriptToExecuteOnDocumentCreated(
            wScript.c_str(),
            Callback<ICoreWebView2AddScriptToExecuteOnDocumentCreatedCompletedHandler>(
                [](HRESULT, LPCWSTR) -> HRESULT { return S_OK; }
            ).Get()
        );
    }
#endif
}

void WebView2Widget::applyDeviceEmulation(int width, int height, double scaleFactor, bool touchEnabled, const QString& userAgent) {
#ifdef _WIN32
    if (m_webview) {
        QString metricsJson = QString(R"({"width":%1,"height":%2,"deviceScaleFactor":%3,"mobile":true})")
            .arg(width).arg(height).arg(scaleFactor);
        m_webview->CallDevToolsProtocolMethod(L"Emulation.setDeviceMetricsOverride", metricsJson.toStdWString().c_str(), nullptr);

        QString touchJson = QString(R"({"enabled":%1,"maxTouchPoints":5})")
            .arg(touchEnabled ? "true" : "false");
        m_webview->CallDevToolsProtocolMethod(L"Emulation.setTouchEmulationEnabled", touchJson.toStdWString().c_str(), nullptr);

        QString mouseTouchJson = QString(R"({"enabled":%1,"configuration":"mobile"})")
            .arg(touchEnabled ? "true" : "false");
        m_webview->CallDevToolsProtocolMethod(L"Emulation.setEmitTouchEventsForMouse", mouseTouchJson.toStdWString().c_str(), nullptr);

        if (!userAgent.isEmpty()) {
            QString uaJson = QString(R"({"userAgent":"%1"})").arg(userAgent);
            m_webview->CallDevToolsProtocolMethod(L"Network.setUserAgentOverride", uaJson.toStdWString().c_str(), nullptr);
        }
    }
#endif
}

bool WebView2Widget::isInitialized() const {
    return m_initialized;
}

QString WebView2Widget::currentUrl() const {
    return m_currentUrl;
}

} // namespace ionexa::engines
