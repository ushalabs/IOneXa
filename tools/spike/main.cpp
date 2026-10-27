#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <wrl.h>
#include <iostream>
#include <string>
#include <vector>

#include "WebView2.h"

using namespace Microsoft::WRL;

// Global WebView2 pointers for spike
static ComPtr<ICoreWebView2Controller> g_controller;
static ComPtr<ICoreWebView2> g_webview;
static HWND g_hwnd = nullptr;
static bool g_navigation_completed = false;
static HRESULT g_navigation_status = E_FAIL;
static bool g_automated_test = false;
static int g_exit_code = 0;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_SIZE:
        if (g_controller) {
            RECT bounds;
            GetClientRect(hWnd, &bounds);
            g_controller->put_Bounds(bounds);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(g_exit_code);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);

    // Attach to parent console if available so CLI logs stream properly
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        FILE* fp;
        freopen_s(&fp, "CONOUT$", "w", stdout);
        freopen_s(&fp, "CONOUT$", "w", stderr);
    }

    // Initialize COM for UI/STA
    HRESULT hrCom = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hrCom)) {
        std::wcerr << L"[IOneXa Spike] CoInitializeEx failed: 0x" << std::hex << hrCom << std::endl;
        return 1;
    }

    std::wstring targetUrl = L"http://127.0.0.1:8081";
    int durationMs = 0;

    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argv) {
        for (int i = 1; i < argc; ++i) {
            if (std::wstring(argv[i]) == L"--url" && i + 1 < argc) {
                targetUrl = argv[++i];
            } else if (std::wstring(argv[i]) == L"--test") {
                g_automated_test = true;
            } else if (std::wstring(argv[i]) == L"--duration" && i + 1 < argc) {
                durationMs = _wtoi(argv[++i]);
            }
        }
        LocalFree(argv);
    }

    const wchar_t CLASS_NAME[] = L"IOneXaWebView2SpikeWindow";

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassW(&wc);

    // Mobile-like dimensions for preview: 390 x 844 (iPhone 14/15 size)
    const int windowWidth = 420;
    const int windowHeight = 880;

    g_hwnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"IOneXa Spike — Expo Web Preview (Not an iOS Runtime)",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        windowWidth, windowHeight,
        nullptr, nullptr, hInstance, nullptr
    );

    if (!g_hwnd) {
        return 1;
    }

    ShowWindow(g_hwnd, g_automated_test ? SW_HIDE : nCmdShow);
    UpdateWindow(g_hwnd);

    // Get temp directory for WebView2 user data folder
    wchar_t tempPath[MAX_PATH];
    GetTempPathW(MAX_PATH, tempPath);
    std::wstring userDataFolder = std::wstring(tempPath) + L"IOneXa_Spike_UserData";
    CreateDirectoryW(userDataFolder.c_str(), nullptr);

    // Create WebView2 Environment
    HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(
        nullptr, userDataFolder.c_str(), nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [targetUrl, durationMs](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
                if (FAILED(result) || !env) {
                    std::wcerr << L"[IOneXa Spike] Failed to create WebView2 environment: 0x" << std::hex << result << std::endl;
                    g_exit_code = 2;
                    DestroyWindow(g_hwnd);
                    return result;
                }

                env->CreateCoreWebView2Controller(
                    g_hwnd,
                    Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [targetUrl, durationMs](HRESULT ctrlResult, ICoreWebView2Controller* controller) -> HRESULT {
                            if (FAILED(ctrlResult) || !controller) {
                                std::wcerr << L"[IOneXa Spike] Failed to create WebView2 controller: 0x" << std::hex << ctrlResult << std::endl;
                                g_exit_code = 3;
                                DestroyWindow(g_hwnd);
                                return ctrlResult;
                            }

                            g_controller = controller;
                            g_controller->get_CoreWebView2(&g_webview);

                            RECT bounds;
                            GetClientRect(g_hwnd, &bounds);
                            g_controller->put_Bounds(bounds);
                            g_controller->put_IsVisible(TRUE);

                            // Listen for navigation completed
                            EventRegistrationToken navToken;
                            g_webview->add_NavigationCompleted(
                                Callback<ICoreWebView2NavigationCompletedEventHandler>(
                                    [durationMs](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
                                        BOOL success = FALSE;
                                        args->get_IsSuccess(&success);
                                        COREWEBVIEW2_WEB_ERROR_STATUS webErrorStatus;
                                        args->get_WebErrorStatus(&webErrorStatus);

                                        g_navigation_completed = true;
                                        std::wcout << L"[IOneXa Spike] Navigation completed. Success: " << (success ? L"TRUE" : L"FALSE") 
                                                  << L", Status: " << (int)webErrorStatus << std::endl;

                                        if (g_automated_test) {
                                            g_exit_code = success ? 0 : 4;
                                            if (durationMs > 0) {
                                                SetTimer(g_hwnd, 1, durationMs, nullptr);
                                            } else {
                                                PostMessage(g_hwnd, WM_CLOSE, 0, 0);
                                            }
                                        }
                                        return S_OK;
                                    }).Get(),
                                &navToken
                            );

                            std::wcout << L"[IOneXa Spike] Navigating to: " << targetUrl << std::endl;
                            g_webview->Navigate(targetUrl.c_str());

                            return S_OK;
                        }).Get()
                );
                return S_OK;
            }).Get()
    );

    if (FAILED(hr)) {
        std::wcerr << L"[IOneXa Spike] Call to CreateCoreWebView2EnvironmentWithOptions failed: 0x" << std::hex << hr << std::endl;
        return 1;
    }

    // Message loop
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        if (msg.message == WM_TIMER && msg.wParam == 1) {
            KillTimer(g_hwnd, 1);
            DestroyWindow(g_hwnd);
            continue;
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    CoUninitialize();
    return (int)msg.wParam;
}
