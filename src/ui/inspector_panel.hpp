#pragma once

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>

#include "project/project_scanner.hpp"
#include "project/compatibility_analyzer.hpp"
#include "process/metro_supervisor.hpp"
#include "engines/fast_refresh_bridge.hpp"

namespace ionexa::ui {

class InspectorPanel : public QWidget {
    Q_OBJECT

public:
    explicit InspectorPanel(QWidget* parent = nullptr);
    ~InspectorPanel() override;

    void updateProject(const project::ProjectDetails& details);
    void updateProcess(int pid, int port, process::MetroStatus status);
    void updateViewport(int width, int height, bool isLandscape, double zoom);
    void updateFastRefresh(const engines::FastRefreshStats& stats, engines::BridgeState state);
    void updateCompatibility(const project::CompatibilityReport& report);
    void updateRendererStatus(const QString& status, bool isHealthy);

    void saveSettings();
    void loadSettings();

signals:
    void handoffExportRequested();

private:
    void setupUi();

    // Section 1: Engine & Honesty
    QLabel* m_lblEngineName{nullptr};
    QLabel* m_lblFidelityDisclosure{nullptr};
    QLabel* m_lblRendererStatus{nullptr};

    // Section 2: Viewport
    QLabel* m_lblDimensions{nullptr};
    QLabel* m_lblOrientation{nullptr};
    QLabel* m_lblZoom{nullptr};

    // Section 3: Process Diagnostics
    QLabel* m_lblProcessStatus{nullptr};
    QLabel* m_lblProcessPid{nullptr};
    QLabel* m_lblProcessPort{nullptr};
    QPushButton* m_btnCopyEndpoint{nullptr};

    // Section 4: Fast Refresh & Live Reload
    QLabel* m_lblHmrStatus{nullptr};
    QLabel* m_lblHmrRefreshCount{nullptr};
    QLabel* m_lblHmrReloadCount{nullptr};
    QLabel* m_lblHmrLastUpdate{nullptr};

    // Section 5: Project Metadata
    QLabel* m_lblProjectName{nullptr};
    QLabel* m_lblExpoSdk{nullptr};
    QLabel* m_lblPackageManager{nullptr};
    QLabel* m_lblWebSupport{nullptr};

    // Section 6: Compatibility Analysis
    QLabel* m_lblCompatScore{nullptr};
    QLabel* m_lblCompatBreakdown{nullptr};
    QLabel* m_lblCompatFiles{nullptr};
    QPushButton* m_btnHandoff{nullptr};

    project::CompatibilityReport m_lastReport;
    project::ProjectDetails m_lastProject;
    int m_lastPort{0};
};

} // namespace ionexa::ui
