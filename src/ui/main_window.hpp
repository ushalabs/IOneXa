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
    void onMetroReady(const QString& url, int port);

private:
    void setupUi();
    void applyDarkTheme();

    std::unique_ptr<core::SessionStateMachine> m_stateMachine;
    std::unique_ptr<process::MetroSupervisor> m_metroSupervisor;
    project::ProjectDetails m_currentProject;

    // UI elements
    QLabel* m_statusBadge{nullptr};
    QLabel* m_truthDisclosureLabel{nullptr};
    QPushButton* m_btnOpen{nullptr};
    QPushButton* m_btnStart{nullptr};
    QPushButton* m_btnStop{nullptr};
    QPushButton* m_btnReload{nullptr};
    QComboBox* m_deviceCombo{nullptr};
    QPushButton* m_btnOrientation{nullptr};
    QWidget* m_canvasContainer{nullptr};
    QWidget* m_deviceViewport{nullptr};
    engines::WebView2Widget* m_webView{nullptr};
    QLabel* m_placeholderLabel{nullptr};
    QTextEdit* m_consoleOutput{nullptr};
    QProgressBar* m_progressBar{nullptr};

    bool m_isLandscape{false};
    int m_deviceWidth{390};
    int m_deviceHeight{844};
};

} // namespace ionexa::ui
