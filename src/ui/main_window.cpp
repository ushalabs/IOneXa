#include "main_window.hpp"
#include "theme.hpp"
#include "project/handoff_generator.hpp"

#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QScrollArea>
#include <QShortcut>
#include <QSplitter>
#include <QStatusBar>
#include <QTabWidget>
#include <QVBoxLayout>

namespace ionexa::ui {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_stateMachine(std::make_unique<core::SessionStateMachine>())
    , m_metroSupervisor(std::make_unique<process::MetroSupervisor>())
    , m_hmrBridge(std::make_unique<engines::FastRefreshBridge>())
    , m_viewportManager(std::make_unique<ViewportManager>())
    , m_networkInspector(std::make_unique<engines::NetworkInspector>()) {

    // Wire Core Engine Signals
    m_stateMachine->setOnStateChanged([this](core::SessionState oldState, core::SessionState newState) {
        QMetaObject::invokeMethod(this, [this, oldState, newState]() {
            updateSessionState(oldState, newState);
        });
    });

    connect(m_metroSupervisor.get(), &process::MetroSupervisor::statusChanged, this, [this](process::MetroStatus status) {
        if (m_inspectorPanel) {
            m_inspectorPanel->updateProcess(m_metroSupervisor->processId(), m_metroSupervisor->activePort(), status);
        }
        updateHomeDashboard();
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
        updateHomeDashboard();
    });

    // Fast Refresh Bridge Signals
    connect(m_hmrBridge.get(), &engines::FastRefreshBridge::reloadRequested, this, [this](bool hard) {
        if (m_webView) {
            m_webView->reload(hard);
        }
        if (m_emulatorWindow) {
            m_emulatorWindow->reload();
        }
    });

    connect(m_hmrBridge.get(), &engines::FastRefreshBridge::fastRefreshDetected, this, [this](const QString& details) {
        appendLog(QString("[Fast Refresh] %1").arg(details));
        statusBar()->showMessage(QString("Fast Refresh: %1").arg(details), 3000);
    });

    connect(m_hmrBridge.get(), &engines::FastRefreshBridge::bundleError, this, [this](const QString& err) {
        appendLog(QString("[Fast Refresh] Bundle error: %1").arg(err), true);
    });

    connect(m_hmrBridge.get(), &engines::FastRefreshBridge::statsUpdated, this, [this](const engines::FastRefreshStats& stats) {
        if (m_inspectorPanel) {
            m_inspectorPanel->updateFastRefresh(stats, m_hmrBridge->state());
        }
    });

    // Viewport Manager Signals
    connect(m_viewportManager.get(), &ViewportManager::viewportChanged, this, [this](int w, int h, bool isLandscape, double zoom) {
        if (m_inspectorPanel) {
            m_inspectorPanel->updateViewport(w, h, isLandscape, zoom);
        }
        if (m_webView) {
            auto prof = m_viewportManager->activeProfile();
            m_webView->applyDeviceEmulation(w, h, prof.scaleFactor, m_viewportManager->isTouchEmulationEnabled(), prof.userAgent);
        }
        updateHomeDashboard();
    });

    // Create Standalone Device Emulator (Window B)
    m_emulatorWindow = new DeviceEmulatorWindow(m_viewportManager.get(), nullptr);
    connect(m_emulatorWindow, &DeviceEmulatorWindow::consoleRequested, this, [this]() {
        if (m_navRail) {
            m_navRail->setActiveSection(NavSection::Console);
        }
        activateWindow();
        raise();
    });
    connect(m_emulatorWindow, &DeviceEmulatorWindow::reloadRequested, this, &MainWindow::onReloadClicked);
    connect(m_emulatorWindow, &DeviceEmulatorWindow::dockRequested, this, &MainWindow::dockEmulator);
    connect(m_emulatorWindow, &DeviceEmulatorWindow::undockRequested, this, &MainWindow::undockEmulator);

    setupUi();
    applyDarkTheme();
    updateSessionState(core::SessionState::Idle, core::SessionState::Idle);

    // Keyboard Shortcuts
    auto* scOpen = new QShortcut(QKeySequence("Ctrl+O"), this);
    connect(scOpen, &QShortcut::activated, this, &MainWindow::onOpenProjectClicked);

    auto* scReload = new QShortcut(QKeySequence("Ctrl+R"), this);
    connect(scReload, &QShortcut::activated, this, &MainWindow::onReloadClicked);

    auto* scClear = new QShortcut(QKeySequence("Ctrl+K"), this);
    connect(scClear, &QShortcut::activated, this, &MainWindow::onClearConsoleClicked);

    auto* scEmulator = new QShortcut(QKeySequence("Ctrl+Shift+E"), this);
    connect(scEmulator, &QShortcut::activated, this, &MainWindow::openEmulatorWindow);

    auto* scInspector = new QShortcut(QKeySequence("Ctrl+I"), this);
    connect(scInspector, &QShortcut::activated, this, &MainWindow::toggleInspector);
}

MainWindow::~MainWindow() {
    if (m_emulatorWindow) {
        m_emulatorWindow->saveSettings();
        delete m_emulatorWindow;
        m_emulatorWindow = nullptr;
    }
}

void MainWindow::setupUi() {
    setWindowTitle("IOneXa — Mobile Development Workbench (Expo/React Native)");
    resize(1360, 860);

    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto* rootLayout = new QVBoxLayout(centralWidget);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // ==========================================
    // 1. TOP COMMAND BAR (Unified Desktop Header)
    // ==========================================
    m_commandBar = new QWidget(centralWidget);
    m_commandBar->setObjectName("commandBar");
    m_commandBar->setFixedHeight(46);
    auto* cmdLayout = new QHBoxLayout(m_commandBar);
    cmdLayout->setContentsMargins(14, 0, 14, 0);
    cmdLayout->setSpacing(8);

    // Brand / Project Monogram Badge
    auto* brandLabel = new QLabel("<b>IOneXa</b>", m_commandBar);
    brandLabel->setStyleSheet("color: #4E9CFF; font-size: 13px; font-weight: 700; padding-right: 6px;");
    cmdLayout->addWidget(brandLabel);

    m_projectBadgeLabel = new QLabel("📁 No Project Loaded", m_commandBar);
    m_projectBadgeLabel->setObjectName("projectBadge");
    m_projectBadgeLabel->setStyleSheet(
        "background-color: #262B35; color: #F2F4F8; border: 1px solid #343B48; "
        "border-radius: 4px; padding: 4px 10px; font-size: 12px; font-weight: 600;"
    );
    cmdLayout->addWidget(m_projectBadgeLabel);

    cmdLayout->addSpacing(8);

    // Execution Controls
    m_btnOpen = new QPushButton("Open Project", m_commandBar);
    m_btnOpen->setObjectName("btnOpen");
    m_btnOpen->setToolTip("Open Expo project folder (Ctrl+O)");
    m_btnOpen->setStyleSheet(Theme::buttonStyle(false));
    connect(m_btnOpen, &QPushButton::clicked, this, &MainWindow::onOpenProjectClicked);
    cmdLayout->addWidget(m_btnOpen);

    m_btnStart = new QPushButton("▶ Start", m_commandBar);
    m_btnStart->setObjectName("btnStart");
    m_btnStart->setToolTip("Start Metro dev server");
    m_btnStart->setStyleSheet(Theme::buttonStyle(true));
    connect(m_btnStart, &QPushButton::clicked, this, &MainWindow::onStartClicked);
    cmdLayout->addWidget(m_btnStart);

    m_btnStop = new QPushButton("⏹ Stop", m_commandBar);
    m_btnStop->setObjectName("btnStop");
    m_btnStop->setToolTip("Stop Metro process");
    m_btnStop->setStyleSheet(Theme::buttonStyle(false, true));
    connect(m_btnStop, &QPushButton::clicked, this, &MainWindow::onStopClicked);
    cmdLayout->addWidget(m_btnStop);

    m_btnReload = new QPushButton("🔄 Reload", m_commandBar);
    m_btnReload->setObjectName("btnReload");
    m_btnReload->setToolTip("Reload Preview (Ctrl+R)");
    m_btnReload->setStyleSheet(Theme::buttonStyle(false));
    connect(m_btnReload, &QPushButton::clicked, this, &MainWindow::onReloadClicked);
    cmdLayout->addWidget(m_btnReload);

    cmdLayout->addSpacing(8);

    // Standalone Emulator Launcher Button
    m_btnEmulator = new QPushButton("📱 Launch Emulator", m_commandBar);
    m_btnEmulator->setObjectName("btnEmulator");
    m_btnEmulator->setToolTip("Launch Standalone Mobile Device Emulator Window (Ctrl+Shift+E)");
    m_btnEmulator->setStyleSheet(
        "QPushButton { background-color: #262B35; color: #4E9CFF; border: 1px solid #4E9CFF; "
        "border-radius: 4px; padding: 5px 12px; font-weight: 600; font-size: 12px; }"
        "QPushButton:hover { background-color: #4E9CFF; color: #FFFFFF; }"
    );
    connect(m_btnEmulator, &QPushButton::clicked, this, &MainWindow::openEmulatorWindow);
    cmdLayout->addWidget(m_btnEmulator);

    cmdLayout->addStretch();

    // Truth In Engineering Badge
    m_truthDisclosureLabel = new QLabel("Expo Web Preview — Not an iOS Runtime", m_commandBar);
    m_truthDisclosureLabel->setObjectName("truthDisclosureBadge");
    m_truthDisclosureLabel->setStyleSheet(
        "color: #F5B84D; background-color: rgba(245, 184, 77, 0.12); "
        "border: 1px solid rgba(245, 184, 77, 0.3); border-radius: 4px; padding: 3px 8px; font-size: 11px; font-weight: 600;"
    );
    cmdLayout->addWidget(m_truthDisclosureLabel);

    // Session Status Pill
    m_statusBadge = new QLabel("Idle", m_commandBar);
    m_statusBadge->setObjectName("statusBadge");
    m_statusBadge->setStyleSheet(Theme::badgeStyle("#262B35", "#A4ADBD"));
    cmdLayout->addWidget(m_statusBadge);

    // Inspector Panel Toggle Button
    m_btnToggleInspector = new QPushButton("📋 Inspector", m_commandBar);
    m_btnToggleInspector->setToolTip("Toggle Session Inspector Panel (Ctrl+I)");
    m_btnToggleInspector->setStyleSheet(Theme::buttonStyle(false));
    connect(m_btnToggleInspector, &QPushButton::clicked, this, &MainWindow::toggleInspector);
    cmdLayout->addWidget(m_btnToggleInspector);

    rootLayout->addWidget(m_commandBar);

    // ==========================================
    // 2. MAIN WORKSPACE BODY
    // ==========================================
    auto* bodyContainer = new QWidget(centralWidget);
    auto* bodyLayout = new QHBoxLayout(bodyContainer);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(0);

    // Left Navigation Rail (52px)
    m_navRail = new NavRail(bodyContainer);
    connect(m_navRail, &NavRail::sectionSelected, this, &MainWindow::onNavSectionSelected);
    bodyLayout->addWidget(m_navRail);

    // Main Central Stack (7 Modular Views)
    m_centralStack = new QStackedWidget(bodyContainer);
    m_centralStack->setObjectName("centralStack");

    // Page 0: Home / Welcome Overview
    m_centralStack->addWidget(createHomeView());
    // Page 1: Projects / Metadata
    m_centralStack->addWidget(createProjectsView());
    // Page 2: Devices / Presets Catalog
    m_centralStack->addWidget(createDevicesView());

    // Page 3: Full Developer Console
    m_developerConsole = new DeveloperConsolePanel(m_centralStack);
    m_centralStack->addWidget(m_developerConsole);

    // Page 4: Full Network Inspector
    m_networkPanel = new NetworkInspectorPanel(m_networkInspector.get(), m_centralStack);
    m_centralStack->addWidget(m_networkPanel);

    // Page 5: Compatibility Analysis
    m_centralStack->addWidget(createCompatibilityView());
    // Page 6: Settings & Preferences
    m_centralStack->addWidget(createSettingsView());

    bodyLayout->addWidget(m_centralStack, 1);

    // Right-Side Collapsible Session Inspector
    m_inspectorPanel = new InspectorPanel(bodyContainer);
    m_inspectorPanel->setFixedWidth(300);
    bodyLayout->addWidget(m_inspectorPanel);

    rootLayout->addWidget(bodyContainer, 1);
}

QWidget* MainWindow::createHomeView() {
    auto* scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("QScrollArea { border: none; background-color: #16181D; }");

    auto* container = new QWidget();
    container->setStyleSheet("background-color: #16181D;");
    auto* layout = new QVBoxLayout(container);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(20);

    // Hero Header
    auto* heroCard = new QFrame(container);
    heroCard->setObjectName("cardFrame");
    heroCard->setStyleSheet(Theme::cardStyle());
    auto* heroLayout = new QVBoxLayout(heroCard);
    heroLayout->setContentsMargins(20, 16, 20, 16);
    heroLayout->setSpacing(8);

    auto* welcomeTitle = new QLabel("Welcome to IOneXa Workbench", heroCard);
    welcomeTitle->setStyleSheet("font-size: 18px; font-weight: 700; color: #F2F4F8;");
    heroLayout->addWidget(welcomeTitle);

    auto* welcomeDesc = new QLabel(
        "Windows-native mobile application testing workbench for Expo and React Native applications. "
        "Separated into a development workbench and an independent top-level device emulator window.",
        heroCard
    );
    welcomeDesc->setWordWrap(true);
    welcomeDesc->setStyleSheet("font-size: 13px; color: #A4ADBD; line-height: 1.4;");
    heroLayout->addWidget(welcomeDesc);

    layout->addWidget(heroCard);

    // 2-Column Cards Grid: Standalone Emulator Card & Metro Status Card
    auto* gridLayout = new QHBoxLayout();
    gridLayout->setSpacing(16);

    // Card A: Standalone Device Emulator
    auto* emulatorCard = new QFrame(container);
    emulatorCard->setObjectName("cardFrame");
    emulatorCard->setStyleSheet(Theme::cardStyle());
    auto* emuLayout = new QVBoxLayout(emulatorCard);
    emuLayout->setContentsMargins(20, 16, 20, 16);
    emuLayout->setSpacing(12);

    auto* emuTitle = new QLabel("📱 Standalone Device Emulator", emulatorCard);
    emuTitle->setStyleSheet("font-size: 15px; font-weight: 600; color: #4E9CFF;");
    emuLayout->addWidget(emuTitle);

    auto* emuDesc = new QLabel(
        "Runs the mobile application preview in an independent, resizable Windows window with "
        "calibrated phone bezels, Dynamic Island, and adjacent hardware controls.",
        emulatorCard
    );
    emuDesc->setWordWrap(true);
    emuDesc->setStyleSheet("font-size: 12px; color: #A4ADBD;");
    emuLayout->addWidget(emuDesc);

    m_homeEmulatorProfile = new QLabel("Device: iPhone 14 / 15 (390 x 844)", emulatorCard);
    m_homeEmulatorProfile->setStyleSheet("font-weight: 600; color: #F2F4F8; font-size: 12px;");
    emuLayout->addWidget(m_homeEmulatorProfile);

    m_homeEmulatorMode = new QLabel("Mode: Floating (Independent Desktop Window)", emulatorCard);
    m_homeEmulatorMode->setStyleSheet("color: #43C990; font-size: 12px; font-weight: 500;");
    emuLayout->addWidget(m_homeEmulatorMode);

    auto* emuBtnLayout = new QHBoxLayout();
    m_homeBtnLaunchEmulator = new QPushButton("📱 Open Emulator Window", emulatorCard);
    m_homeBtnLaunchEmulator->setStyleSheet(Theme::buttonStyle(true));
    connect(m_homeBtnLaunchEmulator, &QPushButton::clicked, this, &MainWindow::openEmulatorWindow);
    emuBtnLayout->addWidget(m_homeBtnLaunchEmulator);

    m_homeBtnDockToggle = new QPushButton("⇲ Dock / Undock", emulatorCard);
    m_homeBtnDockToggle->setStyleSheet(Theme::buttonStyle(false));
    connect(m_homeBtnDockToggle, &QPushButton::clicked, this, [this]() {
        if (m_emulatorWindow) {
            if (m_emulatorWindow->isDocked()) {
                undockEmulator();
            } else {
                dockEmulator();
            }
        }
    });
    emuBtnLayout->addWidget(m_homeBtnDockToggle);

    emuLayout->addLayout(emuBtnLayout);
    gridLayout->addWidget(emulatorCard, 1);

    // Card B: Project & Metro Status
    auto* projectCard = new QFrame(container);
    projectCard->setObjectName("cardFrame");
    projectCard->setStyleSheet(Theme::cardStyle());
    auto* projLayout = new QVBoxLayout(projectCard);
    projLayout->setContentsMargins(20, 16, 20, 16);
    projLayout->setSpacing(12);

    auto* projTitle = new QLabel("⚡ Metro Process & Session", projectCard);
    projTitle->setStyleSheet("font-size: 15px; font-weight: 600; color: #43C990;");
    projLayout->addWidget(projTitle);

    m_homeProjectTitle = new QLabel("Project: None Loaded", projectCard);
    m_homeProjectTitle->setStyleSheet("font-weight: 600; color: #F2F4F8; font-size: 12px;");
    projLayout->addWidget(m_homeProjectTitle);

    m_homeProjectPath = new QLabel("Path: -", projectCard);
    m_homeProjectPath->setStyleSheet("color: #A4ADBD; font-size: 11px;");
    m_homeProjectPath->setWordWrap(true);
    projLayout->addWidget(m_homeProjectPath);

    m_homeProjectSdk = new QLabel("Expo SDK: -", projectCard);
    m_homeProjectSdk->setStyleSheet("color: #A4ADBD; font-size: 11px;");
    projLayout->addWidget(m_homeProjectSdk);

    m_homeMetroStatus = new QLabel("Metro Dev Server: Stopped", projectCard);
    m_homeMetroStatus->setStyleSheet("color: #F06B72; font-size: 12px; font-weight: 500;");
    projLayout->addWidget(m_homeMetroStatus);

    m_homeMetroUrl = new QLabel("Endpoint: Not running", projectCard);
    m_homeMetroUrl->setStyleSheet("color: #6B7688; font-size: 11px; font-family: monospace;");
    projLayout->addWidget(m_homeMetroUrl);

    auto* projBtnLayout = new QHBoxLayout();
    auto* btnQuickOpen = new QPushButton("Open Project...", projectCard);
    btnQuickOpen->setStyleSheet(Theme::buttonStyle(false));
    connect(btnQuickOpen, &QPushButton::clicked, this, &MainWindow::onOpenProjectClicked);
    projBtnLayout->addWidget(btnQuickOpen);

    auto* btnQuickStart = new QPushButton("▶ Start", projectCard);
    btnQuickStart->setStyleSheet(Theme::buttonStyle(true));
    connect(btnQuickStart, &QPushButton::clicked, this, &MainWindow::onStartClicked);
    projBtnLayout->addWidget(btnQuickStart);

    projLayout->addLayout(projBtnLayout);
    gridLayout->addWidget(projectCard, 1);

    layout->addLayout(gridLayout);

    // Collapsible Bottom Dock (Diagnostics / Quick Output)
    auto* bottomDock = new QFrame(container);
    bottomDock->setObjectName("cardFrame");
    bottomDock->setStyleSheet(Theme::cardStyle());
    auto* dockLayout = new QVBoxLayout(bottomDock);
    dockLayout->setContentsMargins(16, 12, 16, 12);

    auto* dockHeaderLayout = new QHBoxLayout();
    auto* dockTitle = new QLabel("💻 Diagnostics & Console Summary", bottomDock);
    dockTitle->setStyleSheet("font-weight: 600; color: #F2F4F8; font-size: 13px;");
    dockHeaderLayout->addWidget(dockTitle);

    dockHeaderLayout->addStretch();

    auto* btnViewFullConsole = new QPushButton("View Full Console →", bottomDock);
    btnViewFullConsole->setStyleSheet(Theme::buttonStyle(false));
    connect(btnViewFullConsole, &QPushButton::clicked, this, [this]() {
        if (m_navRail) m_navRail->setActiveSection(NavSection::Console);
    });
    dockHeaderLayout->addWidget(btnViewFullConsole);
    dockLayout->addLayout(dockHeaderLayout);

    auto* dockHint = new QLabel(
        "Real-time Metro process orchestration, Fast Refresh events, uncaught exceptions, "
        "and browser console logs are aggregated here.",
        bottomDock
    );
    dockHint->setStyleSheet("color: #A4ADBD; font-size: 12px;");
    dockLayout->addWidget(dockHint);

    layout->addWidget(bottomDock);
    layout->addStretch();

    scroll->setWidget(container);
    return scroll;
}

QWidget* MainWindow::createProjectsView() {
    auto* container = new QWidget();
    container->setStyleSheet("background-color: #16181D; padding: 24px;");
    auto* layout = new QVBoxLayout(container);
    layout->setSpacing(16);

    auto* title = new QLabel("Project Metadata & Web Readiness", container);
    title->setStyleSheet("font-size: 18px; font-weight: 700; color: #F2F4F8;");
    layout->addWidget(title);

    auto* card = new QFrame(container);
    card->setObjectName("cardFrame");
    card->setStyleSheet(Theme::cardStyle());
    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(20, 16, 20, 16);
    cardLayout->setSpacing(10);

    m_projName = new QLabel("Project Name: (None)", card);
    m_projName->setStyleSheet("font-size: 14px; font-weight: 600; color: #F2F4F8;");
    cardLayout->addWidget(m_projName);

    m_projPath = new QLabel("Root Directory: (None)", card);
    m_projPath->setStyleSheet("color: #A4ADBD; font-size: 12px;");
    cardLayout->addWidget(m_projPath);

    m_projSdk = new QLabel("Expo SDK Version: -", card);
    m_projSdk->setStyleSheet("color: #A4ADBD; font-size: 12px;");
    cardLayout->addWidget(m_projSdk);

    m_projEntry = new QLabel("Application Entry Point: -", card);
    m_projEntry->setStyleSheet("color: #A4ADBD; font-size: 12px;");
    cardLayout->addWidget(m_projEntry);

    m_projPkgManager = new QLabel("Detected Package Manager: -", card);
    m_projPkgManager->setStyleSheet("color: #A4ADBD; font-size: 12px;");
    cardLayout->addWidget(m_projPkgManager);

    m_projWebSupport = new QLabel("Web Support Packages: Not Verified", card);
    m_projWebSupport->setStyleSheet("color: #F5B84D; font-size: 12px; font-weight: 600;");
    cardLayout->addWidget(m_projWebSupport);

    m_btnRemediateWeb = new QPushButton("Copy Web Remediation Command", card);
    m_btnRemediateWeb->setStyleSheet(Theme::buttonStyle(false));
    m_btnRemediateWeb->setVisible(false);
    connect(m_btnRemediateWeb, &QPushButton::clicked, this, [this]() {
        if (!m_currentProject.remediationCommand.empty()) {
            QApplication::clipboard()->setText(QString::fromStdString(m_currentProject.remediationCommand));
            statusBar()->showMessage("Remediation command copied to clipboard!", 3000);
        }
    });
    cardLayout->addWidget(m_btnRemediateWeb);

    layout->addWidget(card);
    layout->addStretch();
    return container;
}

QWidget* MainWindow::createDevicesView() {
    auto* scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("QScrollArea { border: none; background-color: #16181D; }");

    auto* container = new QWidget();
    container->setStyleSheet("background-color: #16181D; padding: 24px;");
    auto* layout = new QVBoxLayout(container);
    layout->setSpacing(16);

    auto* title = new QLabel("Device Profiles & Emulator Manager", container);
    title->setStyleSheet("font-size: 18px; font-weight: 700; color: #F2F4F8;");
    layout->addWidget(title);

    auto* desc = new QLabel(
        "Select a viewport profile to calibrate the Standalone Mobile Device Emulator (Window B). "
        "Supports dynamic resolution switching, orientation rotation, and high-DPI scaling.",
        container
    );
    desc->setStyleSheet("color: #A4ADBD; font-size: 13px;");
    layout->addWidget(desc);

    const auto& profiles = m_viewportManager->profiles();
    for (int i = 0; i < profiles.size(); ++i) {
        const auto& prof = profiles[i];
        auto* card = new QFrame(container);
        card->setObjectName("cardFrame");
        card->setStyleSheet(Theme::cardStyle());
        auto* cardLayout = new QHBoxLayout(card);
        cardLayout->setContentsMargins(16, 12, 16, 12);

        auto* infoLayout = new QVBoxLayout();
        auto* nameLabel = new QLabel(QString("%1 (%2 x %3)").arg(prof.name).arg(prof.width).arg(prof.height), card);
        nameLabel->setStyleSheet("font-size: 14px; font-weight: 600; color: #F2F4F8;");
        infoLayout->addWidget(nameLabel);

        QString details = QString("Scale: %1x | Dynamic Island: %2 | Type: %3")
            .arg(prof.scaleFactor)
            .arg(prof.hasDynamicIsland ? "Yes" : "No")
            .arg(prof.isMobile ? "Handheld Phone" : "Tablet Viewport");
        auto* detailLabel = new QLabel(details, card);
        detailLabel->setStyleSheet("color: #A4ADBD; font-size: 12px;");
        infoLayout->addWidget(detailLabel);

        cardLayout->addLayout(infoLayout, 1);

        auto* btnSelect = new QPushButton("Select & Launch", card);
        btnSelect->setStyleSheet(Theme::buttonStyle(i == m_viewportManager->activeProfileIndex()));
        connect(btnSelect, &QPushButton::clicked, this, [this, i]() {
            m_viewportManager->selectProfile(i);
            openEmulatorWindow();
        });
        cardLayout->addWidget(btnSelect);

        layout->addWidget(card);
    }

    layout->addStretch();
    scroll->setWidget(container);
    return scroll;
}

QWidget* MainWindow::createCompatibilityView() {
    auto* container = new QWidget();
    container->setStyleSheet("background-color: #16181D; padding: 24px;");
    auto* layout = new QVBoxLayout(container);
    layout->setSpacing(16);

    auto* title = new QLabel("Web Compatibility Analysis & Native iOS Handoff", container);
    title->setStyleSheet("font-size: 18px; font-weight: 700; color: #F2F4F8;");
    layout->addWidget(title);

    auto* card = new QFrame(container);
    card->setObjectName("cardFrame");
    card->setStyleSheet(Theme::cardStyle());
    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(20, 16, 20, 16);
    cardLayout->setSpacing(12);

    m_compatScoreLabel = new QLabel("Web Compatibility Score: 100% (Full Web Readiness)", card);
    m_compatScoreLabel->setStyleSheet("font-size: 14px; font-weight: 600; color: #43C990;");
    cardLayout->addWidget(m_compatScoreLabel);

    m_compatScoreBar = new QProgressBar(card);
    m_compatScoreBar->setRange(0, 100);
    m_compatScoreBar->setValue(100);
    m_compatScoreBar->setStyleSheet(
        "QProgressBar { background-color: #1E222A; border: 1px solid #343B48; border-radius: 4px; height: 16px; text-align: center; color: #FFFFFF; font-weight: 600; font-size: 11px; }"
        "QProgressBar::chunk { background-color: #43C990; border-radius: 3px; }"
    );
    cardLayout->addWidget(m_compatScoreBar);

    m_compatDetailsText = new QTextEdit(card);
    m_compatDetailsText->setReadOnly(true);
    m_compatDetailsText->setStyleSheet(
        "background-color: #1E222A; border: 1px solid #343B48; color: #F2F4F8; "
        "border-radius: 4px; font-family: monospace; font-size: 12px; padding: 8px;"
    );
    m_compatDetailsText->setPlainText("No project analyzed yet. Open an Expo project to run compatibility analysis.");
    m_compatDetailsText->setFixedHeight(180);
    cardLayout->addWidget(m_compatDetailsText);

    m_btnExportHandoff = new QPushButton("📱 Export Physical iPhone & Xcode Testing Guide (Markdown)", card);
    m_btnExportHandoff->setStyleSheet(Theme::buttonStyle(true));
    connect(m_btnExportHandoff, &QPushButton::clicked, this, [this]() {
        if (!m_currentProject.isValid) {
            QMessageBox::information(this, "Handoff", "Please open an Expo project first.");
            return;
        }
        auto guide = project::HandoffGenerator::generateMarkdown(m_currentProject, m_lastCompatReport);
        QString outPath = QString::fromStdString(m_currentProject.canonicalPath) + "/TESTING_ON_PHYSICAL_IPHONE.md";
        QFile file(outPath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write(guide.toUtf8());
            file.close();
            QMessageBox::information(this, "Handoff Guide Exported", QString("Guide successfully generated at:\n%1").arg(outPath));
            appendLog(QString("[Handoff] Exported physical testing guide: %1").arg(outPath));
        }
    });
    cardLayout->addWidget(m_btnExportHandoff);

    layout->addWidget(card);
    layout->addStretch();
    return container;
}

QWidget* MainWindow::createSettingsView() {
    auto* container = new QWidget();
    container->setStyleSheet("background-color: #16181D; padding: 24px;");
    auto* layout = new QVBoxLayout(container);
    layout->setSpacing(16);

    auto* title = new QLabel("Settings & Preferences", container);
    title->setStyleSheet("font-size: 18px; font-weight: 700; color: #F2F4F8;");
    layout->addWidget(title);

    auto* card = new QFrame(container);
    card->setObjectName("cardFrame");
    card->setStyleSheet(Theme::cardStyle());
    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(20, 16, 20, 16);
    cardLayout->setSpacing(12);

    auto* portLabel = new QLabel("Default Metro Server Port: <b>8081</b>", card);
    portLabel->setStyleSheet("color: #F2F4F8; font-size: 13px;");
    cardLayout->addWidget(portLabel);

    auto* hostLabel = new QLabel("Loopback Boundary: <b>127.0.0.1</b> (Localhost only)", card);
    hostLabel->setStyleSheet("color: #F2F4F8; font-size: 13px;");
    cardLayout->addWidget(hostLabel);

    auto* themeLabel = new QLabel("Theme Mode: <b>Dark Mode (Default)</b>", card);
    themeLabel->setStyleSheet("color: #F2F4F8; font-size: 13px;");
    cardLayout->addWidget(themeLabel);

    auto* truthLabel = new QLabel(
        "Truth in Engineering Invariant:\n"
        "IOneXa operates via Expo Web Preview inside Edge WebView2 with Chrome DevTools Protocol emulation. "
        "It does not claim to run native iOS binary code.",
        card
    );
    truthLabel->setStyleSheet("color: #F5B84D; font-size: 12px; line-height: 1.4;");
    truthLabel->setWordWrap(true);
    cardLayout->addWidget(truthLabel);

    layout->addWidget(card);
    layout->addStretch();
    return container;
}

void MainWindow::applyDarkTheme() {
    setStyleSheet(Theme::workbenchStyle());
}

void MainWindow::onNavSectionSelected(NavSection section) {
    int idx = static_cast<int>(section);
    if (m_centralStack && idx >= 0 && idx < m_centralStack->count()) {
        m_centralStack->setCurrentIndex(idx);
    }
}

void MainWindow::toggleInspector() {
    if (m_inspectorPanel) {
        bool visible = !m_inspectorPanel->isVisible();
        m_inspectorPanel->setVisible(visible);
        m_btnToggleInspector->setText(visible ? "📋 Inspector" : "📋 Show Inspector");
    }
}

void MainWindow::updateHomeDashboard() {
    if (m_homeProjectTitle) {
        if (m_currentProject.isValid) {
            m_homeProjectTitle->setText(QString("Project: <b>%1</b>").arg(QString::fromStdString(m_currentProject.projectName)));
            m_homeProjectPath->setText(QString("Path: %1").arg(QString::fromStdString(m_currentProject.canonicalPath)));
            m_homeProjectSdk->setText(QString("Expo SDK: %1 (Manager: %2)").arg(
                QString::fromStdString(m_currentProject.expoSdkVersion),
                QString::fromStdString(m_currentProject.packageManager)
            ));
        } else {
            m_homeProjectTitle->setText("Project: None Loaded");
            m_homeProjectPath->setText("Path: -");
            m_homeProjectSdk->setText("Expo SDK: -");
        }
    }

    if (m_homeMetroStatus && m_metroSupervisor) {
        auto status = m_metroSupervisor->status();
        if (status == process::MetroStatus::Ready) {
            m_homeMetroStatus->setText(QString("Metro Dev Server: Ready (Port %1)").arg(m_metroSupervisor->activePort()));
            m_homeMetroStatus->setStyleSheet("color: #43C990; font-size: 12px; font-weight: 600;");
            m_homeMetroUrl->setText(QString("http://127.0.0.1:%1").arg(m_metroSupervisor->activePort()));
        } else if (status == process::MetroStatus::Starting) {
            m_homeMetroStatus->setText("Metro Dev Server: Starting...");
            m_homeMetroStatus->setStyleSheet("color: #F5B84D; font-size: 12px; font-weight: 600;");
        } else {
            m_homeMetroStatus->setText("Metro Dev Server: Stopped");
            m_homeMetroStatus->setStyleSheet("color: #F06B72; font-size: 12px; font-weight: 500;");
            m_homeMetroUrl->setText("Endpoint: Not running");
        }
    }

    if (m_homeEmulatorProfile && m_viewportManager) {
        auto prof = m_viewportManager->activeProfile();
        m_homeEmulatorProfile->setText(QString("Device: %1 (%2 x %3)").arg(prof.name).arg(prof.width).arg(prof.height));
    }
}

void MainWindow::updateProjectsView() {
    if (!m_projName) return;
    if (m_currentProject.isValid) {
        m_projName->setText(QString("Project Name: %1").arg(QString::fromStdString(m_currentProject.projectName)));
        m_projPath->setText(QString("Root Directory: %1").arg(QString::fromStdString(m_currentProject.canonicalPath)));
        m_projSdk->setText(QString("Expo SDK Version: %1").arg(QString::fromStdString(m_currentProject.expoSdkVersion)));
        m_projEntry->setText(QString("Application Entry Point: %1").arg(QString::fromStdString(m_currentProject.entryPoint)));
        m_projPkgManager->setText(QString("Detected Package Manager: %1").arg(QString::fromStdString(m_currentProject.packageManager)));
        
        if (m_currentProject.hasWebSupport) {
            m_projWebSupport->setText("Web Support Packages: INSTALLED (react-native-web, react-dom, @expo/metro-runtime)");
            m_projWebSupport->setStyleSheet("color: #43C990; font-size: 12px; font-weight: 600;");
            m_btnRemediateWeb->setVisible(false);
        } else {
            m_projWebSupport->setText("Web Support Packages: MISSING WEB PACKAGES");
            m_projWebSupport->setStyleSheet("color: #F06B72; font-size: 12px; font-weight: 600;");
            m_btnRemediateWeb->setVisible(true);
        }
    }
}

void MainWindow::updateCompatibilityView() {
    if (!m_compatScoreLabel) return;
    m_compatScoreLabel->setText(QString("Web Compatibility Score: %1% (%2)")
        .arg(m_lastCompatReport.compatibilityScore, 0, 'f', 1)
        .arg(m_lastCompatReport.tierSummary())
    );
    m_compatScoreBar->setValue(static_cast<int>(std::round(m_lastCompatReport.compatibilityScore)));

    QString details;
    details += QString("Scan Summary: %1 files inspected\n\n").arg(m_lastCompatReport.fileAnalysis.totalSourceFiles);
    for (const auto& rec : m_lastCompatReport.recommendations) {
        details += QString("• %1\n").arg(rec);
    }
    m_compatDetailsText->setPlainText(details);
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
    updateHomeDashboard();
}

void MainWindow::appendLog(const QString& line, bool isStderr) {
    if (m_developerConsole) {
        m_developerConsole->appendLog(
            isStderr ? ConsoleLogLevel::Error : ConsoleLogLevel::Info,
            ConsoleLogSource::Metro,
            line
        );
    }
}

void MainWindow::onZoomChanged(int zoomPercent) {
    if (m_viewportManager) {
        m_viewportManager->setZoomFactor(zoomPercent / 100.0);
    }
    appendLog(QString("[Viewport] Zoom level adjusted to %1%").arg(zoomPercent));
}

void MainWindow::onCopyDiagnosticsClicked() {
    if (m_developerConsole) {
        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setText(m_developerConsole->exportAsText());
        statusBar()->showMessage("Diagnostics copied to clipboard!", 3000);
    }
}

void MainWindow::onClearConsoleClicked() {
    if (m_developerConsole) {
        m_developerConsole->clear();
        statusBar()->showMessage("Console cleared.", 2000);
    }
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

        m_projectBadgeLabel->setText(QString("📁 %1").arg(QString::fromStdString(m_currentProject.projectName)));
        m_projectBadgeLabel->setToolTip(QString("Root: %1").arg(QString::fromStdString(m_currentProject.canonicalPath)));

        appendLog(QString("[Project] Loaded '%1' (Expo SDK: %2, Manager: %3)")
                  .arg(QString::fromStdString(m_currentProject.projectName))
                  .arg(QString::fromStdString(m_currentProject.expoSdkVersion))
                  .arg(QString::fromStdString(m_currentProject.packageManager)));

        if (!m_currentProject.hasWebSupport) {
            appendLog(QString("[Project] Warning: Missing web packages. Run '%1'")
                      .arg(QString::fromStdString(m_currentProject.remediationCommand)), true);
        }

        m_lastCompatReport = project::CompatibilityAnalyzer::analyze(dir);
        if (m_inspectorPanel) {
            m_inspectorPanel->updateCompatibility(m_lastCompatReport);
        }
        updateCompatibilityView();
        updateProjectsView();
        updateHomeDashboard();

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
        if (m_emulatorWindow) {
            m_emulatorWindow->showPlaceholder(true, "Session Stopped\nClick ▶ Start to relaunch Expo preview");
        }
    }
}

void MainWindow::onReloadClicked() {
    appendLog("[Engine] Manual reload requested.");
    if (m_hmrBridge) {
        m_hmrBridge->requestManualReload(false);
    } else if (m_webView) {
        m_webView->reload();
    }
    if (m_emulatorWindow) {
        m_emulatorWindow->reload();
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
    if (m_emulatorWindow) {
        m_emulatorWindow->navigate(url);
        m_emulatorWindow->show();
        m_emulatorWindow->raise();
    }
    if (m_stateMachine->canTransitionTo(core::SessionState::Running)) {
        m_stateMachine->transitionTo(core::SessionState::Running);
    }
    updateHomeDashboard();
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

void MainWindow::openEmulatorWindow() {
    if (m_emulatorWindow) {
        m_emulatorWindow->show();
        m_emulatorWindow->raise();
        m_emulatorWindow->activateWindow();
        appendLog("[Workbench] Mobile Device Emulator window brought to front.");
    }
}

void MainWindow::dockEmulator() {
    if (!m_emulatorWindow) return;
    m_emulatorWindow->hide();
    m_emulatorWindow->setDocked(true);
    appendLog("[Layout] Device emulator docked into Workbench.");
    updateHomeDashboard();
}

void MainWindow::undockEmulator() {
    if (!m_emulatorWindow) return;
    m_emulatorWindow->setDocked(false);
    m_emulatorWindow->show();
    m_emulatorWindow->raise();
    m_emulatorWindow->activateWindow();
    appendLog("[Layout] Device emulator running in standalone window.");
    updateHomeDashboard();
}

} // namespace ionexa::ui
