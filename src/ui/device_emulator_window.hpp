#pragma once

#include <QMainWindow>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QComboBox>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QAction>
#include <QSettings>
#include <QCloseEvent>
#include <QResizeEvent>
#include <memory>

#include "viewport_manager.hpp"
#include "engines/webview2_widget.hpp"
#include "theme.hpp"

namespace ionexa::ui {

enum class EmulatorZoomMode {
    FitWindow,
    Fixed
};

class DeviceEmulatorWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit DeviceEmulatorWindow(ViewportManager* viewportManager, QWidget* parent = nullptr);
    ~DeviceEmulatorWindow() override;

    [[nodiscard]] ViewportManager* viewportManager() const { return m_viewportManager; }
    [[nodiscard]] engines::WebView2Widget* webView() const { return m_webView; }
    [[nodiscard]] bool isAlwaysOnTop() const { return m_alwaysOnTop; }
    [[nodiscard]] bool areBezelsVisible() const { return m_bezelsVisible; }
    [[nodiscard]] bool isDocked() const { return m_isDocked; }
    [[nodiscard]] double currentScale() const { return m_currentScale; }
    [[nodiscard]] EmulatorZoomMode zoomMode() const { return m_zoomMode; }

    void setWebView(engines::WebView2Widget* webView);
    void setAlwaysOnTop(bool onTop);
    void setBezelsVisible(bool visible);
    void setZoomMode(EmulatorZoomMode mode, double fixedScale = 1.0);
    void setDocked(bool docked);
    void updateAppearance();
    void showPlaceholder(bool show, const QString& text = QString());
    void showErrorOverlay(const QString& title, const QString& message, const QString& remediation = QString());
    void hideErrorOverlay();
    void navigate(const QString& url);
    void reload();

    // Screenshot helper
    QString takeScreenshot(bool copyToClipboard = true);

    // Persistence
    void saveSettings();
    void loadSettings();

signals:
    void dockRequested();
    void undockRequested();
    void consoleRequested();
    void reloadRequested();
    void screenshotCaptured(const QString& filePath);
    void emulatorClosed();

protected:
    void resizeEvent(QResizeEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onViewportChanged();
    void onPowerClicked();
    void onReloadClicked();
    void onRotateClicked();
    void onPresetMenuTriggered(QAction* action);
    void onZoomInClicked();
    void onZoomOutClicked();
    void onFitWindowClicked();
    void onActualSizeClicked();
    void onScreenshotClicked();
    void onTouchToggled();
    void onPinToggled();
    void onBezelsToggled();
    void onDockToggled();

private:
    void setupUi();
    void setupToolbar();
    void updateWindowHeader();
    void recalculateScale();
    void updateFrameDimensions();

    ViewportManager* m_viewportManager{nullptr};
    engines::WebView2Widget* m_webView{nullptr};

    // State
    bool m_alwaysOnTop{false};
    bool m_bezelsVisible{true};
    bool m_isDocked{false};
    bool m_screenPoweredOn{true};
    EmulatorZoomMode m_zoomMode{EmulatorZoomMode::FitWindow};
    double m_currentScale{1.0};
    double m_fixedScale{1.0};

    // UI Structure
    QWidget* m_centralContainer{nullptr};
    QWidget* m_canvasArea{nullptr};
    QWidget* m_sideToolbar{nullptr};

    // Device Frame & Chrome
    QFrame* m_deviceFrame{nullptr};
    QVBoxLayout* m_deviceFrameLayout{nullptr};
    QFrame* m_dynamicIsland{nullptr};
    QWidget* m_contentArea{nullptr};
    QVBoxLayout* m_contentLayout{nullptr};
    QLabel* m_placeholderLabel{nullptr};
    QFrame* m_errorOverlay{nullptr};
    QLabel* m_errorTitleLabel{nullptr};
    QLabel* m_errorMessageLabel{nullptr};
    QLabel* m_errorRemediationLabel{nullptr};
    QPushButton* m_btnErrorRetry{nullptr};
    QPushButton* m_btnErrorDismiss{nullptr};
    QFrame* m_homeIndicator{nullptr};

    // Bottom Status & Truth Badge
    QLabel* m_truthDisclosureLabel{nullptr};
    QLabel* m_dimensionStatusLabel{nullptr};

    // Toolbar buttons
    QToolButton* m_btnPower{nullptr};
    QToolButton* m_btnReload{nullptr};
    QToolButton* m_btnRotate{nullptr};
    QToolButton* m_btnPreset{nullptr};
    QMenu* m_presetMenu{nullptr};
    QToolButton* m_btnZoomIn{nullptr};
    QToolButton* m_btnZoomOut{nullptr};
    QToolButton* m_btnFitWindow{nullptr};
    QToolButton* m_btnActualSize{nullptr};
    QToolButton* m_btnScreenshot{nullptr};
    QToolButton* m_btnTouch{nullptr};
    QToolButton* m_btnPin{nullptr};
    QToolButton* m_btnBezels{nullptr};
    QToolButton* m_btnOpenConsole{nullptr};
    QToolButton* m_btnDock{nullptr};
};

} // namespace ionexa::ui
