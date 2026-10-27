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

namespace ionexa::ui {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_stateMachine(std::make_unique<core::SessionStateMachine>()) {
    
    m_stateMachine->setOnStateChanged([this](core::SessionState oldState, core::SessionState newState) {
        // Safe dispatch to Qt thread
        QMetaObject::invokeMethod(this, [this, oldState, newState]() {
            updateSessionState(oldState, newState);
        });
    });

    setupUi();
    applyDarkTheme();
    updateSessionState(core::SessionState::Idle, core::SessionState::Idle);
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi() {
    setWindowTitle("IOneXa — Mobile Development Workbench (Expo/React Native)");
    resize(1280, 850);

    // Central widget & main splitter
    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Top Header / Toolbar Bar
    auto* topHeader = new QWidget(this);
    topHeader->setObjectName("topHeader");
    topHeader->setFixedHeight(50);
    auto* headerLayout = new QHBoxLayout(topHeader);
    headerLayout->setContentsMargins(16, 0, 16, 0);
    headerLayout->setSpacing(12);

    m_btnOpen = new QPushButton("Open Project", topHeader);
    m_btnOpen->setObjectName("btnOpen");
    connect(m_btnOpen, &QPushButton::clicked, this, &MainWindow::onOpenProjectClicked);

    m_btnStart = new QPushButton("▶ Start Preview", topHeader);
    m_btnStart->setObjectName("btnStart");
    connect(m_btnStart, &QPushButton::clicked, this, &MainWindow::onStartClicked);

    m_btnStop = new QPushButton("⏹ Stop", topHeader);
    m_btnStop->setObjectName("btnStop");
    connect(m_btnStop, &QPushButton::clicked, this, &MainWindow::onStopClicked);

    m_btnReload = new QPushButton("🔄 Reload", topHeader);
    m_btnReload->setObjectName("btnReload");
    connect(m_btnReload, &QPushButton::clicked, this, &MainWindow::onReloadClicked);

    headerLayout->addWidget(m_btnOpen);
    headerLayout->addWidget(m_btnStart);
    headerLayout->addWidget(m_btnStop);
    headerLayout->addWidget(m_btnReload);
    headerLayout->addSpacing(16);

    // Device preset selector
    m_deviceCombo = new QComboBox(topHeader);
    m_deviceCombo->addItem("iPhone 14/15 (390 x 844)");
    m_deviceCombo->addItem("iPhone 15 Pro Max (430 x 932)");
    m_deviceCombo->addItem("Pixel 7 (412 x 915)");
    m_deviceCombo->addItem("iPad Mini (744 x 1133)");
    connect(m_deviceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onDevicePresetChanged);

    m_btnOrientation = new QPushButton("🔄 Rotate", topHeader);
    connect(m_btnOrientation, &QPushButton::clicked, this, &MainWindow::onToggleOrientationClicked);

    headerLayout->addWidget(new QLabel("Device:", topHeader));
    headerLayout->addWidget(m_deviceCombo);
    headerLayout->addWidget(m_btnOrientation);

    headerLayout->addStretch();

    // Persistent Truth-in-Labeling Badge
    m_truthDisclosureLabel = new QLabel("Expo Web Preview — Not an iOS Runtime", topHeader);
    m_truthDisclosureLabel->setObjectName("truthDisclosureBadge");
    headerLayout->addWidget(m_truthDisclosureLabel);

    // Status Badge
    m_statusBadge = new QLabel("Idle", topHeader);
    m_statusBadge->setObjectName("statusBadge");
    headerLayout->addWidget(m_statusBadge);

    mainLayout->addWidget(topHeader);

    // Vertical Splitter for Canvas + Bottom Diagnostics
    auto* vSplitter = new QSplitter(Qt::Vertical, centralWidget);
    vSplitter->setContentsMargins(0, 0, 0, 0);

    // Canvas Center Area
    m_canvasContainer = new QWidget(vSplitter);
    m_canvasContainer->setObjectName("canvasContainer");
    auto* canvasLayout = new QVBoxLayout(m_canvasContainer);
    canvasLayout->setAlignment(Qt::AlignCenter);

    // Device Viewport Frame (simulated mobile border)
    m_deviceViewport = new QFrame(m_canvasContainer);
    m_deviceViewport->setObjectName("deviceFrame");
    m_deviceViewport->setFixedSize(m_deviceWidth, m_deviceHeight);

    auto* viewportInnerLayout = new QVBoxLayout(m_deviceViewport);
    viewportInnerLayout->setAlignment(Qt::AlignCenter);

    auto* placeholderLabel = new QLabel("No Project Loaded\nClick 'Open Project' to select an Expo directory", m_deviceViewport);
    placeholderLabel->setAlignment(Qt::AlignCenter);
    placeholderLabel->setObjectName("placeholderLabel");
    viewportInnerLayout->addWidget(placeholderLabel);

    canvasLayout->addWidget(m_deviceViewport);
    vSplitter->addWidget(m_canvasContainer);

    // Bottom Console Panel
    auto* consoleContainer = new QWidget(vSplitter);
    consoleContainer->setObjectName("consoleContainer");
    auto* consoleLayout = new QVBoxLayout(consoleContainer);
    consoleLayout->setContentsMargins(12, 8, 12, 8);

    auto* consoleHeader = new QLabel("Terminal & Runtime Diagnostics", consoleContainer);
    consoleHeader->setObjectName("consoleHeader");
    consoleLayout->addWidget(consoleHeader);

    m_consoleOutput = new QTextEdit(consoleContainer);
    m_consoleOutput->setReadOnly(true);
    m_consoleOutput->setObjectName("consoleOutput");
    consoleLayout->addWidget(m_consoleOutput);

    vSplitter->addWidget(consoleContainer);
    vSplitter->setSizes({600, 200});

    mainLayout->addWidget(vSplitter);

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
            border: 2px solid #334155;
            border-radius: 36px;
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

void MainWindow::onOpenProjectClicked() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Expo Project Directory");
    if (!dir.isEmpty()) {
        appendLog(QString("[Project] Selected directory: %1").arg(dir));
        m_stateMachine->transitionTo(core::SessionState::Inspecting);
        // Simulate inspection completion
        m_stateMachine->transitionTo(core::SessionState::Ready);
    }
}

void MainWindow::onStartClicked() {
    if (m_stateMachine->canTransitionTo(core::SessionState::Starting)) {
        m_stateMachine->transitionTo(core::SessionState::Starting);
        m_stateMachine->transitionTo(core::SessionState::Running);
    }
}

void MainWindow::onStopClicked() {
    if (m_stateMachine->canTransitionTo(core::SessionState::Stopping)) {
        m_stateMachine->transitionTo(core::SessionState::Stopping);
        m_stateMachine->transitionTo(core::SessionState::Stopped);
    }
}

void MainWindow::onReloadClicked() {
    appendLog("[Engine] Reload requested.");
}

void MainWindow::onDevicePresetChanged(int index) {
    switch (index) {
    case 0: m_deviceWidth = 390; m_deviceHeight = 844; break;
    case 1: m_deviceWidth = 430; m_deviceHeight = 932; break;
    case 2: m_deviceWidth = 412; m_deviceHeight = 915; break;
    case 3: m_deviceWidth = 744; m_deviceHeight = 1133; break;
    default: m_deviceWidth = 390; m_deviceHeight = 844; break;
    }

    int w = m_isLandscape ? m_deviceHeight : m_deviceWidth;
    int h = m_isLandscape ? m_deviceWidth : m_deviceHeight;
    m_deviceViewport->setFixedSize(w, h);
    appendLog(QString("[Viewport] Resized to %1 x %2").arg(w).arg(h));
}

void MainWindow::onToggleOrientationClicked() {
    m_isLandscape = !m_isLandscape;
    int w = m_isLandscape ? m_deviceHeight : m_deviceWidth;
    int h = m_isLandscape ? m_deviceWidth : m_deviceHeight;
    m_deviceViewport->setFixedSize(w, h);
    appendLog(QString("[Viewport] Orientation toggled: %1 (%2 x %3)")
              .arg(m_isLandscape ? "Landscape" : "Portrait").arg(w).arg(h));
}

} // namespace ionexa::ui
