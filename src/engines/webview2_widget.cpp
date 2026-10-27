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

void WebView2Widget::reload() {
#ifdef _WIN32
    if (m_webview) {
        m_webview->Reload();
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

bool WebView2Widget::isInitialized() const {
    return m_initialized;
}

QString WebView2Widget::currentUrl() const {
    return m_currentUrl;
}

} // namespace ionexa::engines
