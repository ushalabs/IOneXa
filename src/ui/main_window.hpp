#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QComboBox>
#include <QProgressBar>
#include <memory>

#include "core/session_state.hpp"
#include "project/project_scanner.hpp"
#include "process/metro_supervisor.hpp"
#include "engines/webview2_widget.hpp"
#include "engines/fast_refresh_bridge.hpp"
#include "nav_rail.hpp"
#include "inspector_panel.hpp"
#include "viewport_manager.hpp"
#include "developer_console_panel.hpp"

namespace ionexa::ui {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    void updateSessionState(core::SessionState oldState, core::SessionState newState);
    void appendLog(const QString& line, bool isStderr = false);

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

private:
    void setupUi();
    void applyDarkTheme();

    std::unique_ptr<core::SessionStateMachine> m_stateMachine;
    std::unique_ptr<process::MetroSupervisor> m_metroSupervisor;
    std::unique_ptr<engines::FastRefreshBridge> m_hmrBridge;
    std::unique_ptr<ViewportManager> m_viewportManager;
    project::ProjectDetails m_currentProject;

    // UI elements
    NavRail* m_navRail{nullptr};
    InspectorPanel* m_inspectorPanel{nullptr};
    QLabel* m_statusBadge{nullptr};
    QLabel* m_truthDisclosureLabel{nullptr};
    QPushButton* m_btnOpen{nullptr};
    QPushButton* m_btnStart{nullptr};
    QPushButton* m_btnStop{nullptr};
    QPushButton* m_btnReload{nullptr};
    QComboBox* m_deviceCombo{nullptr};
    QPushButton* m_btnOrientation{nullptr};
    QPushButton* m_btnTouch{nullptr};
    QComboBox* m_zoomCombo{nullptr};
    QPushButton* m_btnCopyDiagnostics{nullptr};
    QPushButton* m_btnClearConsole{nullptr};
    QWidget* m_canvasContainer{nullptr};
    QWidget* m_deviceViewport{nullptr};
    QFrame* m_dynamicIsland{nullptr};
    QFrame* m_homeIndicator{nullptr};
    engines::WebView2Widget* m_webView{nullptr};
    QLabel* m_placeholderLabel{nullptr};
    DeveloperConsolePanel* m_developerConsole{nullptr};
    QProgressBar* m_progressBar{nullptr};
};

} // namespace ionexa::ui
