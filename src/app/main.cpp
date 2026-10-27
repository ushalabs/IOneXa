#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include <QApplication>
#include <iostream>

#include "ui/main_window.hpp"

int main(int argc, char* argv[]) {
    // Initialize COM Apartment for WebView2 compatibility
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        std::cerr << "Warning: CoInitializeEx returned 0x" << std::hex << hr << std::endl;
    }

    QApplication app(argc, argv);
    app.setApplicationName("IOneXa");
    app.setOrganizationName("UshaLabs");
    app.setApplicationVersion("0.1.0");

    ionexa::ui::MainWindow mainWindow;

    // Check for automated headless or self-test flag
    bool autoExit = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--self-test") {
            autoExit = true;
            break;
        }
    }

    if (autoExit) {
        mainWindow.show();
        std::cout << "[IOneXa Self-Test] Application initialized successfully." << std::endl;
        QMetaObject::invokeMethod(&app, "quit", Qt::QueuedConnection);
    } else {
        mainWindow.show();
    }

    int exitCode = app.exec();

    CoUninitialize();
    return exitCode;
}
