#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QComboBox>
#include <QProgressBar>
#include <QStackedWidget>
#include <QSplitter>
#include <memory>

#include "core/session_state.hpp"
#include "project/project_scanner.hpp"
#include "project/compatibility_analyzer.hpp"
#include "process/metro_supervisor.hpp"
#include "engines/webview2_widget.hpp"
#include "engines/fast_refresh_bridge.hpp"
#include "nav_rail.hpp"
#include "inspector_panel.hpp"
#include "viewport_manager.hpp"
#include "developer_console_panel.hpp"
#include "network_inspector_panel.hpp"
#include "device_emulator_window.hpp"
#include "device_manager_panel.hpp"
#include "engines/network_inspector.hpp"

namespace ionexa::ui {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    void updateSessionState(core::SessionState oldState, core::SessionState newState);
    void appendLog(const QString& line, bool isStderr = false);

    [[nodiscard]] QStackedWidget* centralStack() const { return m_centralStack; }
    [[nodiscard]] NavRail* navRail() const { return m_navRail; }
    [[nodiscard]] DeviceEmulatorWindow* emulatorWindow() const { return m_emulatorWindow; }
    [[nodiscard]] InspectorPanel* inspectorPanel() const { return m_inspectorPanel; }

public slots:
    void openEmulatorWindow();
    void dockEmulator();
    void undockEmulator();
    void toggleInspector();

private slots:
    void onOpenProjectClicked();
    void onStartClicked();
    void onStopClicked();
    void onReloadClicked();
    void onDevicePresetChanged(int index);
    void onToggleOrientationClicked();
    void onZoomChanged(int zoomPercent);
    void onCopyDiagnosticsClicked();
    void onClearConsoleClicked();
    void onMetroReady(const QString& url, int port);
    void onNavSectionSelected(NavSection section);

private:
    void setupUi();
    void applyDarkTheme();

    // View generators
    QWidget* createHomeView();
    QWidget* createProjectsView();
    QWidget* createDevicesView();
    QWidget* createCompatibilityView();
    QWidget* createSettingsView();

    void updateHomeDashboard();
    void updateProjectsView();
    void updateCompatibilityView();

    std::unique_ptr<core::SessionStateMachine> m_stateMachine;
    std::unique_ptr<process::MetroSupervisor> m_metroSupervisor;
    std::unique_ptr<engines::FastRefreshBridge> m_hmrBridge;
    std::unique_ptr<ViewportManager> m_viewportManager;
    std::unique_ptr<engines::NetworkInspector> m_networkInspector;
    project::ProjectDetails m_currentProject;
    project::CompatibilityReport m_lastCompatReport;
    QMap<QString, qint64> m_pendingNetworkRequests;

    // Command Bar elements
    QWidget* m_commandBar{nullptr};
    QLabel* m_projectBadgeLabel{nullptr};
    QLabel* m_statusBadge{nullptr};
    QLabel* m_truthDisclosureLabel{nullptr};
    QPushButton* m_btnOpen{nullptr};
    QPushButton* m_btnStart{nullptr};
    QPushButton* m_btnStop{nullptr};
    QPushButton* m_btnReload{nullptr};
    QPushButton* m_btnEmulator{nullptr};
    QPushButton* m_btnToggleInspector{nullptr};

    // Navigation & Workspace Structure
    NavRail* m_navRail{nullptr};
    QStackedWidget* m_centralStack{nullptr};
    InspectorPanel* m_inspectorPanel{nullptr};
    DeviceEmulatorWindow* m_emulatorWindow{nullptr};

    // Home View elements
    QLabel* m_homeProjectTitle{nullptr};
    QLabel* m_homeProjectPath{nullptr};
    QLabel* m_homeProjectSdk{nullptr};
    QLabel* m_homeMetroStatus{nullptr};
    QLabel* m_homeMetroUrl{nullptr};
    QLabel* m_homeEmulatorProfile{nullptr};
    QLabel* m_homeEmulatorMode{nullptr};
    QPushButton* m_homeBtnLaunchEmulator{nullptr};
    QPushButton* m_homeBtnDockToggle{nullptr};

    // Projects View elements
    QLabel* m_projName{nullptr};
    QLabel* m_projPath{nullptr};
    QLabel* m_projSdk{nullptr};
    QLabel* m_projEntry{nullptr};
    QLabel* m_projPkgManager{nullptr};
    QLabel* m_projWebSupport{nullptr};
    QPushButton* m_btnRemediateWeb{nullptr};

    // Devices View elements
    QVector<QPushButton*> m_deviceCardLaunchButtons;

    // Compatibility View elements
    QLabel* m_compatScoreLabel{nullptr};
    QProgressBar* m_compatScoreBar{nullptr};
    QTextEdit* m_compatDetailsText{nullptr};
    QPushButton* m_btnExportHandoff{nullptr};

    // Embedded / Shared panels
    DeveloperConsolePanel* m_developerConsole{nullptr};
    NetworkInspectorPanel* m_networkPanel{nullptr};
    QTabWidget* m_bottomTabs{nullptr};

    // Docked canvas viewport
    QWidget* m_dockedCanvasContainer{nullptr};
    QWidget* m_deviceViewport{nullptr};
    QFrame* m_dynamicIsland{nullptr};
    QFrame* m_homeIndicator{nullptr};
    engines::WebView2Widget* m_webView{nullptr};
    QLabel* m_placeholderLabel{nullptr};
};

} // namespace ionexa::ui
