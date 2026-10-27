#include "inspector_panel.hpp"
#include <QGroupBox>
#include <QFormLayout>
#include <QScrollArea>

namespace ionexa::ui {

InspectorPanel::InspectorPanel(QWidget* parent)
    : QWidget(parent) {
    setFixedWidth(280);
    setObjectName("inspectorPanel");
    setupUi();
}

void InspectorPanel::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(12);

    auto* header = new QLabel("SESSION INSPECTOR", this);
    header->setStyleSheet("color: #38bdf8; font-weight: 700; font-size: 11px; letter-spacing: 1px;");
    mainLayout->addWidget(header);

    // Group 1: Runtime Engine & Truth Disclosure
    auto* grpEngine = new QGroupBox("Runtime Engine", this);
    auto* formEngine = new QFormLayout(grpEngine);
    m_lblEngineName = new QLabel("Expo Web (WebView2)", grpEngine);
    m_lblFidelityDisclosure = new QLabel("Web Preview (Not iOS)", grpEngine);
    m_lblFidelityDisclosure->setStyleSheet("color: #fde047; font-weight: 600; font-size: 11px;");
    formEngine->addRow("Target:", m_lblEngineName);
    formEngine->addRow("Fidelity:", m_lblFidelityDisclosure);
    mainLayout->addWidget(grpEngine);

    // Group 2: Viewport Dimensions
    auto* grpViewport = new QGroupBox("Viewport Frame", this);
    auto* formViewport = new QFormLayout(grpViewport);
    m_lblDimensions = new QLabel("390 x 844 px", grpViewport);
    m_lblOrientation = new QLabel("Portrait", grpViewport);
    m_lblZoom = new QLabel("100%", grpViewport);
    formViewport->addRow("Size:", m_lblDimensions);
    formViewport->addRow("Mode:", m_lblOrientation);
    formViewport->addRow("Scale:", m_lblZoom);
    mainLayout->addWidget(grpViewport);

    // Group 3: Process Diagnostics
    auto* grpProcess = new QGroupBox("Process Health", this);
    auto* formProcess = new QFormLayout(grpProcess);
    m_lblProcessStatus = new QLabel("Idle", grpProcess);
    m_lblProcessPid = new QLabel("—", grpProcess);
    m_lblProcessPort = new QLabel("—", grpProcess);
    formProcess->addRow("State:", m_lblProcessStatus);
    formProcess->addRow("PID:", m_lblProcessPid);
    formProcess->addRow("Port:", m_lblProcessPort);
    mainLayout->addWidget(grpProcess);

    // Group 4: Fast Refresh & Live Reload
    auto* grpHmr = new QGroupBox("Fast Refresh & Live Reload", this);
    auto* formHmr = new QFormLayout(grpHmr);
    m_lblHmrStatus = new QLabel("Idle", grpHmr);
    m_lblHmrRefreshCount = new QLabel("0", grpHmr);
    m_lblHmrReloadCount = new QLabel("0", grpHmr);
    m_lblHmrLastUpdate = new QLabel("—", grpHmr);
    formHmr->addRow("Status:", m_lblHmrStatus);
    formHmr->addRow("Refreshes:", m_lblHmrRefreshCount);
    formHmr->addRow("Reloads:", m_lblHmrReloadCount);
    formHmr->addRow("Last Active:", m_lblHmrLastUpdate);
    mainLayout->addWidget(grpHmr);

    // Group 5: Project Information
    auto* grpProject = new QGroupBox("Project Metadata", this);
    auto* formProject = new QFormLayout(grpProject);
    m_lblProjectName = new QLabel("None", grpProject);
    m_lblExpoSdk = new QLabel("—", grpProject);
    m_lblPackageManager = new QLabel("—", grpProject);
    m_lblWebSupport = new QLabel("—", grpProject);
    formProject->addRow("App:", m_lblProjectName);
    formProject->addRow("SDK:", m_lblExpoSdk);
    formProject->addRow("Manager:", m_lblPackageManager);
    formProject->addRow("Web Ready:", m_lblWebSupport);
    mainLayout->addWidget(grpProject);

    mainLayout->addStretch();

    setStyleSheet(R"(
        #inspectorPanel {
            background-color: #0b0f19;
            border-left: 1px solid #1e293b;
        }
        QGroupBox {
            color: #94a3b8;
            font-size: 12px;
            font-weight: 600;
            border: 1px solid #1e293b;
            border-radius: 8px;
            margin-top: 12px;
            padding-top: 10px;
            background-color: #0f172a;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 6px;
            color: #38bdf8;
        }
        QLabel {
            color: #cbd5e1;
            font-size: 11px;
        }
    )");
}

void InspectorPanel::updateProject(const project::ProjectDetails& details) {
    if (details.isValid) {
        m_lblProjectName->setText(QString::fromStdString(details.projectName));
        m_lblExpoSdk->setText(QString::fromStdString(details.expoSdkVersion));
        m_lblPackageManager->setText(QString::fromStdString(details.packageManager));
        m_lblWebSupport->setText(details.hasWebSupport ? "Verified" : "Missing Web Packages");
        m_lblWebSupport->setStyleSheet(details.hasWebSupport ? "color: #4ade80;" : "color: #f87171;");
    } else {
        m_lblProjectName->setText("None");
        m_lblExpoSdk->setText("—");
        m_lblPackageManager->setText("—");
        m_lblWebSupport->setText("—");
    }
}

void InspectorPanel::updateProcess(int pid, int port, process::MetroStatus status) {
    m_lblProcessPid->setText(pid > 0 ? QString::number(pid) : "—");
    m_lblProcessPort->setText(port > 0 ? QString::number(port) : "—");

    switch (status) {
    case process::MetroStatus::Starting:
        m_lblProcessStatus->setText("Starting...");
        m_lblProcessStatus->setStyleSheet("color: #fde047;");
        break;
    case process::MetroStatus::Ready:
        m_lblProcessStatus->setText("Live Ready");
        m_lblProcessStatus->setStyleSheet("color: #4ade80;");
        break;
    case process::MetroStatus::Stopping:
        m_lblProcessStatus->setText("Stopping...");
        m_lblProcessStatus->setStyleSheet("color: #fde047;");
        break;
    case process::MetroStatus::Stopped:
        m_lblProcessStatus->setText("Stopped");
        m_lblProcessStatus->setStyleSheet("color: #94a3b8;");
        break;
    case process::MetroStatus::Error:
        m_lblProcessStatus->setText("Error");
        m_lblProcessStatus->setStyleSheet("color: #f87171;");
        break;
    default:
        m_lblProcessStatus->setText("Idle");
        m_lblProcessStatus->setStyleSheet("color: #94a3b8;");
        break;
    }
}

void InspectorPanel::updateViewport(int width, int height, bool isLandscape, double zoom) {
    m_lblDimensions->setText(QString("%1 x %2 px").arg(width).arg(height));
    m_lblOrientation->setText(isLandscape ? "Landscape" : "Portrait");
    m_lblZoom->setText(QString("%1%").arg(static_cast<int>(zoom * 100)));
}

void InspectorPanel::updateFastRefresh(const engines::FastRefreshStats& stats, engines::BridgeState state) {
    switch (state) {
    case engines::BridgeState::Connecting:
        m_lblHmrStatus->setText("Connecting...");
        m_lblHmrStatus->setStyleSheet("color: #fde047;");
        break;
    case engines::BridgeState::Connected:
        m_lblHmrStatus->setText("Connected (Active)");
        m_lblHmrStatus->setStyleSheet("color: #4ade80;");
        break;
    case engines::BridgeState::Reconnecting:
        m_lblHmrStatus->setText("Reconnecting...");
        m_lblHmrStatus->setStyleSheet("color: #fb923c;");
        break;
    case engines::BridgeState::Stale:
        m_lblHmrStatus->setText("Stale (Disconnected)");
        m_lblHmrStatus->setStyleSheet("color: #f87171;");
        break;
    default:
        m_lblHmrStatus->setText("Idle");
        m_lblHmrStatus->setStyleSheet("color: #94a3b8;");
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

} // namespace ionexa::ui
