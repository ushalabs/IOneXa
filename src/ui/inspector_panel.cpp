#include "inspector_panel.hpp"
#include "theme.hpp"

#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QFile>
#include <QFormLayout>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QToolTip>

#include "project/handoff_generator.hpp"

namespace ionexa::ui {

InspectorPanel::InspectorPanel(QWidget* parent)
    : QWidget(parent) {
    setFixedWidth(290);
    setObjectName("inspectorPanel");
    setupUi();
    loadSettings();
}

InspectorPanel::~InspectorPanel() {
    saveSettings();
}

void InspectorPanel::setupUi() {
    setStyleSheet(
        "QWidget#inspectorPanel { background-color: #1A1D24; border-left: 1px solid #343B48; }"
        "QGroupBox { font-size: 11px; font-weight: 700; color: #A4ADBD; border: 1px solid #343B48; border-radius: 6px; margin-top: 10px; padding: 10px 8px 8px 8px; }"
        "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; left: 10px; padding: 0 4px; background-color: #1A1D24; }"
        "QLabel { color: #F2F4F8; font-size: 11px; }"
    );

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // Header Title
    auto* header = new QWidget(this);
    header->setFixedHeight(36);
    header->setStyleSheet("background-color: #1E222A; border-bottom: 1px solid #2A303C;");
    auto* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(12, 0, 12, 0);

    auto* titleLabel = new QLabel("SESSION INSPECTOR", header);
    titleLabel->setStyleSheet("color: #4E9CFF; font-weight: 700; font-size: 11px; letter-spacing: 0.5px;");
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();

    rootLayout->addWidget(header);

    // Scroll Area for all groups
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("QScrollArea { border: none; background-color: transparent; }");

    auto* container = new QWidget();
    container->setStyleSheet("background-color: transparent;");
    auto* mainLayout = new QVBoxLayout(container);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // Group 1: Runtime Engine & Truth Disclosure
    auto* grpEngine = new QGroupBox("Runtime Engine", container);
    auto* formEngine = new QFormLayout(grpEngine);
    formEngine->setSpacing(6);

    m_lblEngineName = new QLabel("Expo Web (Edge WebView2)", grpEngine);
    m_lblFidelityDisclosure = new QLabel("Web Preview — Not iOS", grpEngine);
    m_lblFidelityDisclosure->setStyleSheet("color: #F5B84D; font-weight: 600; font-size: 11px;");
    m_lblRendererStatus = new QLabel("Active", grpEngine);
    m_lblRendererStatus->setStyleSheet("color: #43C990; font-weight: 600; font-size: 11px;");

    formEngine->addRow("Target:", m_lblEngineName);
    formEngine->addRow("Fidelity:", m_lblFidelityDisclosure);
    formEngine->addRow("Renderer:", m_lblRendererStatus);
    mainLayout->addWidget(grpEngine);

    // Group 2: Viewport Dimensions
    auto* grpViewport = new QGroupBox("Active Viewport", container);
    auto* formViewport = new QFormLayout(grpViewport);
    formViewport->setSpacing(6);

    m_lblDimensions = new QLabel("390 x 844 px", grpViewport);
    m_lblOrientation = new QLabel("Portrait", grpViewport);
    m_lblZoom = new QLabel("100%", grpViewport);

    formViewport->addRow("Size:", m_lblDimensions);
    formViewport->addRow("Mode:", m_lblOrientation);
    formViewport->addRow("Scale:", m_lblZoom);
    mainLayout->addWidget(grpViewport);

    // Group 3: Process Diagnostics
    auto* grpProcess = new QGroupBox("Metro Process", container);
    auto* formProcess = new QFormLayout(grpProcess);
    formProcess->setSpacing(6);

    m_lblProcessStatus = new QLabel("Stopped", grpProcess);
    m_lblProcessStatus->setStyleSheet("color: #F06B72; font-weight: 600;");
    m_lblProcessPid = new QLabel("—", grpProcess);
    m_lblProcessPort = new QLabel("—", grpProcess);

    formProcess->addRow("State:", m_lblProcessStatus);
    formProcess->addRow("PID:", m_lblProcessPid);
    formProcess->addRow("Port:", m_lblProcessPort);

    m_btnCopyEndpoint = new QPushButton("Copy Endpoint (URL)", grpProcess);
    m_btnCopyEndpoint->setStyleSheet(Theme::buttonStyle(false));
    connect(m_btnCopyEndpoint, &QPushButton::clicked, this, [this]() {
        if (m_lastPort > 0) {
            QString url = QString("http://127.0.0.1:%1").arg(m_lastPort);
            QApplication::clipboard()->setText(url);
            QToolTip::showText(m_btnCopyEndpoint->mapToGlobal(QPoint(0, 0)), "Copied: " + url);
        }
    });
    formProcess->addRow("", m_btnCopyEndpoint);
    mainLayout->addWidget(grpProcess);

    // Group 4: Fast Refresh & Live Reload
    auto* grpHmr = new QGroupBox("Fast Refresh (HMR)", container);
    auto* formHmr = new QFormLayout(grpHmr);
    formHmr->setSpacing(6);

    m_lblHmrStatus = new QLabel("Idle", grpHmr);
    m_lblHmrRefreshCount = new QLabel("0", grpHmr);
    m_lblHmrReloadCount = new QLabel("0", grpHmr);
    m_lblHmrLastUpdate = new QLabel("—", grpHmr);

    formHmr->addRow("Socket:", m_lblHmrStatus);
    formHmr->addRow("Refreshes:", m_lblHmrRefreshCount);
    formHmr->addRow("Full Reloads:", m_lblHmrReloadCount);
    formHmr->addRow("Last Applied:", m_lblHmrLastUpdate);
    mainLayout->addWidget(grpHmr);

    // Group 5: Project Information
    auto* grpProject = new QGroupBox("Project Metadata", container);
    auto* formProject = new QFormLayout(grpProject);
    formProject->setSpacing(6);

    m_lblProjectName = new QLabel("None Loaded", grpProject);
    m_lblProjectName->setStyleSheet("font-weight: 600; color: #F2F4F8;");
    m_lblExpoSdk = new QLabel("—", grpProject);
    m_lblPackageManager = new QLabel("—", grpProject);
    m_lblWebSupport = new QLabel("—", grpProject);

    formProject->addRow("Project:", m_lblProjectName);
    formProject->addRow("Expo SDK:", m_lblExpoSdk);
    formProject->addRow("Manager:", m_lblPackageManager);
    formProject->addRow("Web Ready:", m_lblWebSupport);
    mainLayout->addWidget(grpProject);

    // Group 6: Compatibility Analysis
    auto* grpCompat = new QGroupBox("Web Compatibility", container);
    auto* formCompat = new QFormLayout(grpCompat);
    formCompat->setSpacing(6);

    m_lblCompatScore = new QLabel("—", grpCompat);
    m_lblCompatScore->setStyleSheet("font-weight: 700; color: #43C990;");
    m_lblCompatBreakdown = new QLabel("—", grpCompat);
    m_lblCompatFiles = new QLabel("—", grpCompat);

    formCompat->addRow("Web Score:", m_lblCompatScore);
    formCompat->addRow("Modules:", m_lblCompatBreakdown);
    formCompat->addRow("Files:", m_lblCompatFiles);

    m_btnHandoff = new QPushButton("📱 Export iOS Handoff", grpCompat);
    m_btnHandoff->setStyleSheet(Theme::buttonStyle(true));
    connect(m_btnHandoff, &QPushButton::clicked, this, [this]() {
        emit handoffExportRequested();
        if (m_lastProject.isValid) {
            QString md = project::HandoffGenerator::generateMarkdown(m_lastProject, m_lastReport);
            QString savePath = QFileDialog::getSaveFileName(this, "Save iOS Verification Guide", "IOS_VERIFICATION_HANDOFF.md", "Markdown (*.md)");
            if (!savePath.isEmpty()) {
                QFile file(savePath);
                if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    file.write(md.toUtf8());
                    file.close();
                }
            }
        }
    });
    formCompat->addRow("", m_btnHandoff);
    mainLayout->addWidget(grpCompat);

    mainLayout->addStretch();
    scroll->setWidget(container);
    rootLayout->addWidget(scroll, 1);
}

void InspectorPanel::updateProject(const project::ProjectDetails& details) {
    m_lastProject = details;
    m_lblProjectName->setText(details.isValid ? QString::fromStdString(details.projectName) : "None");
    m_lblExpoSdk->setText(details.isValid ? QString::fromStdString(details.expoSdkVersion) : "—");
    m_lblPackageManager->setText(details.isValid ? QString::fromStdString(details.packageManager) : "—");

    if (details.isValid) {
        if (details.hasWebSupport) {
            m_lblWebSupport->setText("Installed");
            m_lblWebSupport->setStyleSheet("color: #43C990; font-weight: 600;");
        } else {
            m_lblWebSupport->setText("Missing Packages");
            m_lblWebSupport->setStyleSheet("color: #F06B72; font-weight: 600;");
        }
    } else {
        m_lblWebSupport->setText("—");
        m_lblWebSupport->setStyleSheet("color: #A4ADBD;");
    }
}

void InspectorPanel::updateProcess(int pid, int port, process::MetroStatus status) {
    m_lastPort = port;
    m_lblProcessPid->setText(pid > 0 ? QString::number(pid) : "—");
    m_lblProcessPort->setText(port > 0 ? QString::number(port) : "—");

    switch (status) {
        case process::MetroStatus::Ready:
            m_lblProcessStatus->setText("Ready");
            m_lblProcessStatus->setStyleSheet("color: #43C990; font-weight: 600;");
            break;
        case process::MetroStatus::Starting:
            m_lblProcessStatus->setText("Starting...");
            m_lblProcessStatus->setStyleSheet("color: #F5B84D; font-weight: 600;");
            break;
        case process::MetroStatus::Stopping:
            m_lblProcessStatus->setText("Stopping...");
            m_lblProcessStatus->setStyleSheet("color: #F5B84D; font-weight: 600;");
            break;
        case process::MetroStatus::Error:
            m_lblProcessStatus->setText("Error");
            m_lblProcessStatus->setStyleSheet("color: #F06B72; font-weight: 600;");
            break;
        case process::MetroStatus::Stopped:
        default:
            m_lblProcessStatus->setText("Stopped");
            m_lblProcessStatus->setStyleSheet("color: #6B7688; font-weight: 600;");
            break;
    }
}

void InspectorPanel::updateViewport(int width, int height, bool isLandscape, double zoom) {
    m_lblDimensions->setText(QString("%1 x %2 px").arg(width).arg(height));
    m_lblOrientation->setText(isLandscape ? "Landscape" : "Portrait");
    m_lblZoom->setText(QString("%1%").arg(static_cast<int>(std::round(zoom * 100.0))));
}

void InspectorPanel::updateFastRefresh(const engines::FastRefreshStats& stats, engines::BridgeState state) {
    switch (state) {
        case engines::BridgeState::Connected:
            m_lblHmrStatus->setText("Connected");
            m_lblHmrStatus->setStyleSheet("color: #43C990; font-weight: 600;");
            break;
        case engines::BridgeState::Connecting:
            m_lblHmrStatus->setText("Connecting...");
            m_lblHmrStatus->setStyleSheet("color: #F5B84D; font-weight: 600;");
            break;
        case engines::BridgeState::Reconnecting:
            m_lblHmrStatus->setText("Reconnecting...");
            m_lblHmrStatus->setStyleSheet("color: #F5B84D; font-weight: 600;");
            break;
        case engines::BridgeState::Stale:
            m_lblHmrStatus->setText("Stale");
            m_lblHmrStatus->setStyleSheet("color: #F06B72; font-weight: 600;");
            break;
        case engines::BridgeState::Idle:
        default:
            m_lblHmrStatus->setText("Idle / Disconnected");
            m_lblHmrStatus->setStyleSheet("color: #6B7688; font-weight: 600;");
            break;
    }

    m_lblHmrRefreshCount->setText(QString::number(stats.fastRefreshCount));
    m_lblHmrReloadCount->setText(QString::number(stats.totalReloads));

    if (stats.lastRefreshTime.isValid()) {
        m_lblHmrLastUpdate->setText(stats.lastRefreshTime.toString("hh:mm:ss"));
    } else {
        m_lblHmrLastUpdate->setText("—");
    }
}

void InspectorPanel::updateCompatibility(const project::CompatibilityReport& report) {
    m_lastReport = report;
    m_lblCompatScore->setText(QString("%1% (%2)").arg(report.compatibilityScore, 0, 'f', 1).arg(report.tierSummary()));

    QString breakdown = QString("%1 safe, %2 poly, %3 adv, %4 unsup")
        .arg(report.supportedCount)
        .arg(report.partialCount)
        .arg(report.unverifiedCount)
        .arg(report.unsupportedCount);
    m_lblCompatBreakdown->setText(breakdown);

    QString files = QString("%1 total (%2 iOS-only, %3 web-only)")
        .arg(report.fileAnalysis.totalSourceFiles)
        .arg(report.fileAnalysis.iosSpecificFiles)
        .arg(report.fileAnalysis.webSpecificFiles);
    m_lblCompatFiles->setText(files);
}

void InspectorPanel::updateRendererStatus(const QString& status, bool isHealthy) {
    if (m_lblRendererStatus) {
        m_lblRendererStatus->setText(status);
        m_lblRendererStatus->setStyleSheet(isHealthy ? "color: #43C990; font-weight: 600;" : "color: #F06B72; font-weight: 600;");
    }
}

void InspectorPanel::saveSettings() {
    QSettings settings("UshaLabs", "IOneXa");
    settings.beginGroup("inspector");
    settings.setValue("width", width());
    settings.setValue("visible", isVisible());
    settings.endGroup();
}

void InspectorPanel::loadSettings() {
    QSettings settings("UshaLabs", "IOneXa");
    settings.beginGroup("inspector");
    int savedW = settings.value("width", 290).toInt();
    if (savedW >= 240 && savedW <= 500) {
        setFixedWidth(savedW);
    }
    settings.endGroup();
}

} // namespace ionexa::ui
