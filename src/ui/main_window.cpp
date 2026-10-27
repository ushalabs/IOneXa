#include "main_window.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QFileDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QMenuBar>
#include <QToolBar>
#include <QFrame>
#include <QClipboard>
#include <QApplication>
#include <QShortcut>
#include <QKeySequence>

namespace ionexa::ui {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_stateMachine(std::make_unique<core::SessionStateMachine>())
    , m_metroSupervisor(std::make_unique<process::MetroSupervisor>(this))
    , m_hmrBridge(std::make_unique<engines::FastRefreshBridge>(this))
    , m_viewportManager(std::make_unique<ViewportManager>(this)) {
    
    m_stateMachine->setOnStateChanged([this](core::SessionState oldState, core::SessionState newState) {
        QMetaObject::invokeMethod(this, [this, oldState, newState]() {
            updateSessionState(oldState, newState);
        });
    });

    connect(m_metroSupervisor.get(), &process::MetroSupervisor::statusChanged, this, [this](process::MetroStatus status) {
        if (m_inspectorPanel) {
            m_inspectorPanel->updateProcess(m_metroSupervisor->processId(), m_metroSupervisor->activePort(), status);
        }
    });

    connect(m_metroSupervisor.get(), &process::MetroSupervisor::ready, this, &MainWindow::onMetroReady);
    connect(m_metroSupervisor.get(), &process::MetroSupervisor::logOutput, this, [this](const QString& line, bool isStderr) {
        appendLog(line, isStderr);
        if (m_hmrBridge) {
            m_hmrBridge->processMetroLog(line);
        }
    });
    connect(m_metroSupervisor.get(), &process::MetroSupervisor::processTerminated, this, [this](int exitCode) {
        appendLog(QString("[Supervisor] Process exited with code %1").arg(exitCode));
        if (m_hmrBridge) {
            m_hmrBridge->markDevServerStopped();
        }
        if (m_stateMachine->canTransitionTo(core::SessionState::Stopped)) {
            m_stateMachine->transitionTo(core::SessionState::Stopped);
        }
    });

    // Fast Refresh Bridge Signals
    connect(m_hmrBridge.get(), &engines::FastRefreshBridge::reloadRequested, this, [this](bool hard) {
        if (m_webView) {
            m_webView->reload(hard);
        }
    });

    connect(m_hmrBridge.get(), &engines::FastRefreshBridge::fastRefreshDetected, this, [this](const QString& details) {
        appendLog(QString("[Fast Refresh] %1").arg(details));
        statusBar()->showMessage(QString("Fast Refresh: %1").arg(details), 3000);
    });

    connect(m_hmrBridge.get(), &engines::FastRefreshBridge::bundleError, this, [this](const QString& err) {
        appendLog(QString("[Bundle Error] %1").arg(err), true);
        statusBar()->showMessage("Bundle compilation error!", 5000);
    });

    connect(m_hmrBridge.get(), &engines::FastRefreshBridge::staleSessionDetected, this, [this](const QString& reason) {
        appendLog(QString("[Live Reload] Stale session: %1").arg(reason), true);
        statusBar()->showMessage("Dev server offline — Reconnecting...", 5000);
    });

    connect(m_hmrBridge.get(), &engines::FastRefreshBridge::reconnected, this, [this]() {
        appendLog("[Live Reload] Metro dev server connection restored. Preview reloaded.");
        statusBar()->showMessage("Dev server reconnected!", 3000);
    });

    connect(m_hmrBridge.get(), &engines::FastRefreshBridge::statsUpdated, this, [this](const engines::FastRefreshStats& stats) {
        if (m_inspectorPanel && m_hmrBridge) {
            m_inspectorPanel->updateFastRefresh(stats, m_hmrBridge->state());
        }
    });

    connect(m_hmrBridge.get(), &engines::FastRefreshBridge::stateChanged, this, [this](engines::BridgeState state) {
        if (m_inspectorPanel && m_hmrBridge) {
            m_inspectorPanel->updateFastRefresh(m_hmrBridge->stats(), state);
        }
    });

    // Viewport Manager Signals
    connect(m_viewportManager.get(), &ViewportManager::viewportChanged, this, [this](int w, int h, bool isLandscape, double zoom) {
        if (m_deviceViewport) {
            m_deviceViewport->setFixedSize(w, h);
        }
        if (m_inspectorPanel) {
            m_inspectorPanel->updateViewport(w, h, isLandscape, zoom);
        }
        if (m_webView) {
            m_webView->setZoomFactor(zoom);
            auto prof = m_viewportManager->activeProfile();
            m_webView->applyDeviceEmulation(w, h, prof.scaleFactor, m_viewportManager->isTouchEmulationEnabled(), prof.userAgent);
        }
        auto prof = m_viewportManager->activeProfile();
        if (m_dynamicIsland) {
            m_dynamicIsland->setVisible(prof.hasDynamicIsland && !isLandscape);
        }
        if (m_homeIndicator) {
            m_homeIndicator->setVisible(!isLandscape);
        }
    });

    connect(m_viewportManager.get(), &ViewportManager::touchEmulationToggled, this, [this](bool enabled) {
        if (m_webView) {
            auto prof = m_viewportManager->activeProfile();
            int w = m_viewportManager->currentPixelWidth();
            int h = m_viewportManager->currentPixelHeight();
            m_webView->applyDeviceEmulation(w, h, prof.scaleFactor, enabled, prof.userAgent);
        }
    });

    setupUi();
    applyDarkTheme();
    updateSessionState(core::SessionState::Idle, core::SessionState::Idle);

    // Keyboard shortcuts
    auto* scOpen = new QShortcut(QKeySequence("Ctrl+O"), this);
    connect(scOpen, &QShortcut::activated, this, &MainWindow::onOpenProjectClicked);

    auto* scReload = new QShortcut(QKeySequence("Ctrl+R"), this);
    connect(scReload, &QShortcut::activated, this, &MainWindow::onReloadClicked);

    auto* scClear = new QShortcut(QKeySequence("Ctrl+K"), this);
    connect(scClear, &QShortcut::activated, this, &MainWindow::onClearConsoleClicked);
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi() {
    setWindowTitle("IOneXa — Mobile Development Workbench (Expo/React Native)");
    resize(1380, 880);

    // Central widget & root layout (Horizontal: NavRail | Workspace | Inspector)
    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto* rootLayout = new QHBoxLayout(centralWidget);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // 1. Navigation Rail (Left, 64px)
    m_navRail = new NavRail(centralWidget);
    rootLayout->addWidget(m_navRail);

    // 2. Middle Container (Toolbar + Canvas + Console)
    auto* middleContainer = new QWidget(centralWidget);
    auto* middleLayout = new QVBoxLayout(middleContainer);
    middleLayout->setContentsMargins(0, 0, 0, 0);
    middleLayout->setSpacing(0);

    // Top Header / Toolbar Bar
    auto* topHeader = new QWidget(middleContainer);
    topHeader->setObjectName("topHeader");
    topHeader->setFixedHeight(50);
    auto* headerLayout = new QHBoxLayout(topHeader);
    headerLayout->setContentsMargins(16, 0, 16, 0);
    headerLayout->setSpacing(10);

    m_btnOpen = new QPushButton("Open Project", topHeader);
    m_btnOpen->setObjectName("btnOpen");
    m_btnOpen->setToolTip("Open Expo project folder (Ctrl+O)");
    connect(m_btnOpen, &QPushButton::clicked, this, &MainWindow::onOpenProjectClicked);

    m_btnStart = new QPushButton("▶ Start", topHeader);
    m_btnStart->setObjectName("btnStart");
    connect(m_btnStart, &QPushButton::clicked, this, &MainWindow::onStartClicked);

    m_btnStop = new QPushButton("⏹ Stop", topHeader);
    m_btnStop->setObjectName("btnStop");
    connect(m_btnStop, &QPushButton::clicked, this, &MainWindow::onStopClicked);

    m_btnReload = new QPushButton("🔄 Reload", topHeader);
    m_btnReload->setObjectName("btnReload");
    m_btnReload->setToolTip("Reload web view (Ctrl+R)");
    connect(m_btnReload, &QPushButton::clicked, this, &MainWindow::onReloadClicked);

    headerLayout->addWidget(m_btnOpen);
    headerLayout->addWidget(m_btnStart);
    headerLayout->addWidget(m_btnStop);
    headerLayout->addWidget(m_btnReload);
    headerLayout->addSpacing(10);

    // Device preset selector
    m_deviceCombo = new QComboBox(topHeader);
    for (const auto& prof : m_viewportManager->profiles()) {
        m_deviceCombo->addItem(QString("%1 (%2 x %3)").arg(prof.name).arg(prof.width).arg(prof.height), prof.id);
    }
    connect(m_deviceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onDevicePresetChanged);

    m_btnOrientation = new QPushButton("🔄 Rotate", topHeader);
    connect(m_btnOrientation, &QPushButton::clicked, this, &MainWindow::onToggleOrientationClicked);

    // Touch emulation toggle
    m_btnTouch = new QPushButton("👆 Touch: ON", topHeader);
    m_btnTouch->setCheckable(true);
    m_btnTouch->setChecked(true);
    connect(m_btnTouch, &QPushButton::clicked, this, [this](bool checked) {
        m_viewportManager->setTouchEmulation(checked);
        m_btnTouch->setText(checked ? "👆 Touch: ON" : "🖱 Touch: OFF");
    });

    // Zoom selector
    m_zoomCombo = new QComboBox(topHeader);
    m_zoomCombo->addItem("100%", 100);
    m_zoomCombo->addItem("75%", 75);
    m_zoomCombo->addItem("125%", 125);
    m_zoomCombo->addItem("150%", 150);
    connect(m_zoomCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) {
        int zoomPercent = m_zoomCombo->itemData(idx).toInt();
        onZoomChanged(zoomPercent);
    });

    headerLayout->addWidget(new QLabel("Device:", topHeader));
    headerLayout->addWidget(m_deviceCombo);
    headerLayout->addWidget(m_btnOrientation);
    headerLayout->addWidget(m_btnTouch);
    headerLayout->addWidget(new QLabel("Zoom:", topHeader));
    headerLayout->addWidget(m_zoomCombo);

    headerLayout->addStretch();

    // Persistent Truth-in-Labeling Badge
    m_truthDisclosureLabel = new QLabel("Expo Web Preview — Not an iOS Runtime", topHeader);
    m_truthDisclosureLabel->setObjectName("truthDisclosureBadge");
    headerLayout->addWidget(m_truthDisclosureLabel);

    // Status Badge
    m_statusBadge = new QLabel("Idle", topHeader);
    m_statusBadge->setObjectName("statusBadge");
    headerLayout->addWidget(m_statusBadge);

    middleLayout->addWidget(topHeader);

    // Vertical Splitter for Canvas + Bottom Diagnostics
    auto* vSplitter = new QSplitter(Qt::Vertical, middleContainer);
    vSplitter->setContentsMargins(0, 0, 0, 0);

    // Canvas Center Area
    m_canvasContainer = new QWidget(vSplitter);
    m_canvasContainer->setObjectName("canvasContainer");
    auto* canvasLayout = new QVBoxLayout(m_canvasContainer);
    canvasLayout->setAlignment(Qt::AlignCenter);

    // Device Viewport Frame (simulated mobile border)
    m_deviceViewport = new QFrame(m_canvasContainer);
    m_deviceViewport->setObjectName("deviceFrame");
    m_deviceViewport->setFixedSize(m_viewportManager->currentPixelWidth(), m_viewportManager->currentPixelHeight());

    auto* viewportInnerLayout = new QVBoxLayout(m_deviceViewport);
    viewportInnerLayout->setContentsMargins(10, 8, 10, 8);
    viewportInnerLayout->setSpacing(4);

    // Dynamic Island Simulation Pill
    m_dynamicIsland = new QFrame(m_deviceViewport);
    m_dynamicIsland->setObjectName("dynamicIsland");
    m_dynamicIsland->setFixedSize(110, 24);
    auto* islandLayout = new QHBoxLayout(m_dynamicIsland);
    islandLayout->setContentsMargins(6, 0, 6, 0);
    auto* cameraDot = new QLabel(m_dynamicIsland);
    cameraDot->setFixedSize(8, 8);
    cameraDot->setStyleSheet("background-color: #0f172a; border-radius: 4px; border: 1px solid #334155;");
    islandLayout->addStretch();
    islandLayout->addWidget(cameraDot);
    viewportInnerLayout->addWidget(m_dynamicIsland, 0, Qt::AlignHCenter);

    m_placeholderLabel = new QLabel("No Project Loaded\nClick 'Open Project' to select an Expo directory", m_deviceViewport);
    m_placeholderLabel->setAlignment(Qt::AlignCenter);
    m_placeholderLabel->setObjectName("placeholderLabel");
    viewportInnerLayout->addWidget(m_placeholderLabel, 1);

    m_webView = new engines::WebView2Widget(m_deviceViewport);
    m_webView->hide();
    viewportInnerLayout->addWidget(m_webView, 1);

    // Home Indicator Simulation Bar
    m_homeIndicator = new QFrame(m_deviceViewport);
    m_homeIndicator->setObjectName("homeIndicator");
    m_homeIndicator->setFixedSize(120, 4);
    viewportInnerLayout->addWidget(m_homeIndicator, 0, Qt::AlignHCenter);

    connect(m_webView, &engines::WebView2Widget::navigationCompleted, this, [this](bool success, int errorCode) {
        if (success) {
            appendLog("[Renderer] Navigation completed successfully.");
        } else {
            appendLog(QString("[Renderer] Navigation failed with error status: %1").arg(errorCode), true);
        }
    });

    connect(m_webView, &engines::WebView2Widget::rendererCrashed, this, [this](int reason) {
        appendLog(QString("[Renderer] CRITICAL: WebView2 renderer process crashed (reason=%1)").arg(reason), true);
    });

    connect(m_webView, &engines::WebView2Widget::webMessageReceived, this, [this](const QString& msg) {
        if (m_hmrBridge) {
            m_hmrBridge->processWebMessage(msg);
        }
    });

    canvasLayout->addWidget(m_deviceViewport);
    vSplitter->addWidget(m_canvasContainer);

    // Bottom Console Panel
    auto* consoleContainer = new QWidget(vSplitter);
    consoleContainer->setObjectName("consoleContainer");
    auto* consoleLayout = new QVBoxLayout(consoleContainer);
    consoleLayout->setContentsMargins(12, 6, 12, 6);

    auto* consoleTopBar = new QHBoxLayout();
    auto* consoleHeader = new QLabel("TERMINAL & RUNTIME DIAGNOSTICS", consoleContainer);
    consoleHeader->setObjectName("consoleHeader");

    m_btnCopyDiagnostics = new QPushButton("Copy Diagnostics", consoleContainer);
    m_btnCopyDiagnostics->setFixedHeight(26);
    connect(m_btnCopyDiagnostics, &QPushButton::clicked, this, &MainWindow::onCopyDiagnosticsClicked);

    m_btnClearConsole = new QPushButton("Clear (Ctrl+K)", consoleContainer);
    m_btnClearConsole->setFixedHeight(26);
    connect(m_btnClearConsole, &QPushButton::clicked, this, &MainWindow::onClearConsoleClicked);

    consoleTopBar->addWidget(consoleHeader);
    consoleTopBar->addStretch();
    consoleTopBar->addWidget(m_btnCopyDiagnostics);
    consoleTopBar->addWidget(m_btnClearConsole);

    consoleLayout->addLayout(consoleTopBar);

    m_consoleOutput = new QTextEdit(consoleContainer);
    m_consoleOutput->setReadOnly(true);
    m_consoleOutput->setObjectName("consoleOutput");
    consoleLayout->addWidget(m_consoleOutput);

    vSplitter->addWidget(consoleContainer);
    vSplitter->setSizes({600, 200});

    middleLayout->addWidget(vSplitter);
    rootLayout->addWidget(middleContainer, 1);

    // 3. Right Inspector Panel (280px)
    m_inspectorPanel = new InspectorPanel(centralWidget);
    rootLayout->addWidget(m_inspectorPanel);

    statusBar()->showMessage("IOneXa Ready — Visual Studio 2022 / Qt 6.8 / Edge WebView2");
}

void MainWindow::applyDarkTheme() {
    setStyleSheet(R"(
        QMainWindow {
            background-color: #0b0f19;
        }
        #topHeader {
            background-color: #0f172a;
            border-bottom: 1px solid #1e293b;
        }
        QPushButton {
            background-color: #1e293b;
            color: #f8fafc;
            border: 1px solid #334155;
            border-radius: 6px;
            padding: 6px 14px;
            font-size: 13px;
            font-weight: 500;
        }
        QPushButton:hover {
            background-color: #334155;
            border-color: #475569;
        }
        QPushButton:pressed {
            background-color: #0f172a;
        }
        #btnStart {
            background-color: #0284c7;
            border-color: #0369a1;
        }
        #btnStart:hover {
            background-color: #0369a1;
        }
        #btnStop {
            background-color: #b91c1c;
            border-color: #991b1b;
        }
        #btnStop:hover {
            background-color: #991b1b;
        }
        QComboBox {
            background-color: #1e293b;
            color: #f8fafc;
            border: 1px solid #334155;
            border-radius: 6px;
            padding: 4px 10px;
        }
        #truthDisclosureBadge {
            background-color: #451a03;
            color: #fde047;
            border: 1px solid #854d0e;
            border-radius: 6px;
            padding: 4px 10px;
            font-weight: 600;
            font-size: 11px;
        }
        #statusBadge {
            background-color: #1e293b;
            color: #38bdf8;
            border: 1px solid #334155;
            border-radius: 6px;
            padding: 4px 12px;
            font-weight: 600;
            font-size: 12px;
        }
        #canvasContainer {
            background-color: #030712;
        }
        #deviceFrame {
            background-color: #020617;
            border: 4px solid #334155;
            border-radius: 36px;
        }
        #dynamicIsland {
            background-color: #000000;
            border-radius: 12px;
            border: 1px solid #1e293b;
        }
        #homeIndicator {
            background-color: #64748b;
            border-radius: 2px;
        }
        #placeholderLabel {
            color: #64748b;
            font-size: 14px;
            font-weight: 500;
        }
        #consoleContainer {
            background-color: #090d16;
            border-top: 1px solid #1e293b;
        }
        #consoleHeader {
            color: #94a3b8;
            font-weight: 600;
            font-size: 12px;
            text-transform: uppercase;
        }
        #consoleOutput {
            background-color: #020617;
            color: #e2e8f0;
            border: 1px solid #1e293b;
            border-radius: 6px;
            font-family: 'Cascadia Code', 'Consolas', monospace;
            font-size: 12px;
        }
        QStatusBar {
            background-color: #0f172a;
            color: #94a3b8;
            border-top: 1px solid #1e293b;
        }
    )");
}

void MainWindow::updateSessionState(core::SessionState oldState, core::SessionState newState) {
    Q_UNUSED(oldState);
    QString stateStr = QString::fromStdString(core::sessionStateToString(newState));
    m_statusBadge->setText(stateStr);

    m_btnOpen->setEnabled(newState == core::SessionState::Idle || newState == core::SessionState::Ready || newState == core::SessionState::Stopped);
    m_btnStart->setEnabled(newState == core::SessionState::Ready || newState == core::SessionState::Stopped);
    m_btnStop->setEnabled(newState == core::SessionState::Running || newState == core::SessionState::Starting);
    m_btnReload->setEnabled(newState == core::SessionState::Running);

    appendLog(QString("[Session] Transitioned to %1").arg(stateStr));
}

void MainWindow::appendLog(const QString& line, bool isStderr) {
    if (isStderr) {
        m_consoleOutput->append(QString("<span style='color: #ef4444;'>%1</span>").arg(line.toHtmlEscaped()));
    } else {
        m_consoleOutput->append(QString("<span style='color: #cbd5e1;'>%1</span>").arg(line.toHtmlEscaped()));
    }
}

void MainWindow::onZoomChanged(int zoomPercent) {
    if (m_viewportManager) {
        m_viewportManager->setZoomFactor(zoomPercent / 100.0);
    }
    appendLog(QString("[Viewport] Zoom level adjusted to %1%").arg(zoomPercent));
}

void MainWindow::onCopyDiagnosticsClicked() {
    QString diagnostics = m_consoleOutput->toPlainText();
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(diagnostics);
    statusBar()->showMessage("Diagnostics copied to clipboard!", 3000);
    appendLog("[Diagnostics] Full log buffer copied to system clipboard.");
}

void MainWindow::onClearConsoleClicked() {
    m_consoleOutput->clear();
    statusBar()->showMessage("Console cleared.", 2000);
}

void MainWindow::onOpenProjectClicked() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Expo Project Directory");
    if (!dir.isEmpty()) {
        appendLog(QString("[Project] Selected directory: %1").arg(dir));
        m_stateMachine->transitionTo(core::SessionState::Inspecting);
        
        m_currentProject = project::ProjectScanner::scan(dir.toStdString());
        if (m_inspectorPanel) {
            m_inspectorPanel->updateProject(m_currentProject);
        }

        if (!m_currentProject.isValid) {
            appendLog(QString("[Project] Error: %1").arg(QString::fromStdString(m_currentProject.errorMessage)), true);
            m_stateMachine->transitionTo(core::SessionState::Error);
            return;
        }

        if (!m_currentProject.isExpoProject) {
            appendLog("[Project] Notice: Directory is not an Expo project.", true);
            m_stateMachine->transitionTo(core::SessionState::Error);
            return;
        }

        appendLog(QString("[Project] Loaded '%1' (Expo SDK: %2, Manager: %3)")
                  .arg(QString::fromStdString(m_currentProject.projectName))
                  .arg(QString::fromStdString(m_currentProject.expoSdkVersion))
                  .arg(QString::fromStdString(m_currentProject.packageManager)));

        if (!m_currentProject.hasWebSupport) {
            appendLog(QString("[Project] Warning: Missing web packages. Run '%1'")
                      .arg(QString::fromStdString(m_currentProject.remediationCommand)), true);
        }

        m_stateMachine->transitionTo(core::SessionState::Ready);
    }
}

void MainWindow::onStartClicked() {
    if (!m_currentProject.isValid || !m_currentProject.isExpoProject) {
        appendLog("[Supervisor] Cannot start: No valid Expo project selected.", true);
        return;
    }

    if (m_stateMachine->canTransitionTo(core::SessionState::Starting)) {
        m_stateMachine->transitionTo(core::SessionState::Starting);
        m_metroSupervisor->start(m_currentProject);
    }
}

void MainWindow::onStopClicked() {
    if (m_stateMachine->canTransitionTo(core::SessionState::Stopping)) {
        m_stateMachine->transitionTo(core::SessionState::Stopping);
        if (m_hmrBridge) {
            m_hmrBridge->markDevServerStopped();
        }
        m_metroSupervisor->stop();
        m_stateMachine->transitionTo(core::SessionState::Stopped);
    }
}

void MainWindow::onReloadClicked() {
    appendLog("[Engine] Manual reload requested.");
    if (m_hmrBridge) {
        m_hmrBridge->requestManualReload(false);
    } else if (m_webView) {
        m_webView->reload();
    }
}

void MainWindow::onMetroReady(const QString& url, int port) {
    appendLog(QString("[Engine] Metro dev server confirmed ready at %1 (port %2)").arg(url).arg(port));
    if (m_hmrBridge) {
        m_hmrBridge->setMetroEndpoint("127.0.0.1", port);
        m_hmrBridge->markDevServerStarted(port);
    }
    if (m_placeholderLabel) m_placeholderLabel->hide();
    if (m_webView) {
        m_webView->show();
        m_webView->navigate(url);
    }
    if (m_stateMachine->canTransitionTo(core::SessionState::Running)) {
        m_stateMachine->transitionTo(core::SessionState::Running);
    }
}

void MainWindow::onDevicePresetChanged(int index) {
    if (m_viewportManager) {
        m_viewportManager->selectProfile(index);
    }
}

void MainWindow::onToggleOrientationClicked() {
    if (m_viewportManager) {
        m_viewportManager->toggleOrientation();
    }
}

} // namespace ionexa::ui
