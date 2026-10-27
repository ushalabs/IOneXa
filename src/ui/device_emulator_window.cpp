#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "device_emulator_window.hpp"

#include <QApplication>
#include <QClipboard>
#include <QDateTime>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QScreen>
#include <QStandardPaths>
#include <QStatusBar>
#include <QToolTip>
#include <cmath>

namespace ionexa::ui {

DeviceEmulatorWindow::DeviceEmulatorWindow(ViewportManager* viewportManager, QWidget* parent)
    : QMainWindow(parent)
    , m_viewportManager(viewportManager) {
    setWindowTitle("IOneXa — Mobile Device Emulator");
    setMinimumSize(420, 600);

    setupUi();
    setupToolbar();
    loadSettings();
    updateAppearance();

    if (m_viewportManager) {
        connect(m_viewportManager, &ViewportManager::viewportChanged, this, &DeviceEmulatorWindow::onViewportChanged);
        connect(m_viewportManager, &ViewportManager::profileChanged, this, [this](const DeviceProfile&) {
            onViewportChanged();
        });
        connect(m_viewportManager, &ViewportManager::touchEmulationToggled, this, [this](bool enabled) {
            if (m_btnTouch) {
                m_btnTouch->setChecked(enabled);
                m_btnTouch->setToolTip(enabled ? "Touch Emulation: ON (Click to disable)" : "Touch Emulation: OFF (Click to enable)");
            }
        });
    }

    onViewportChanged();
}

DeviceEmulatorWindow::~DeviceEmulatorWindow() {
    saveSettings();
}

void DeviceEmulatorWindow::setupUi() {
    setStyleSheet(Theme::emulatorStyle());

    m_centralContainer = new QWidget(this);
    setCentralWidget(m_centralContainer);

    auto* mainLayout = new QHBoxLayout(m_centralContainer);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 1. Central Device Canvas Area
    auto* canvasContainer = new QWidget(m_centralContainer);
    auto* canvasVLayout = new QVBoxLayout(canvasContainer);
    canvasVLayout->setContentsMargins(0, 0, 0, 0);
    canvasVLayout->setSpacing(0);

    m_canvasArea = new QWidget(canvasContainer);
    m_canvasArea->setObjectName("canvasContainer");
    auto* canvasCenteringLayout = new QVBoxLayout(m_canvasArea);
    canvasCenteringLayout->setAlignment(Qt::AlignCenter);
    canvasCenteringLayout->setContentsMargins(20, 20, 20, 20);

    // Mobile Device Frame
    m_deviceFrame = new QFrame(m_canvasArea);
    m_deviceFrame->setObjectName("emulatorDeviceFrame");
    m_deviceFrameLayout = new QVBoxLayout(m_deviceFrame);
    m_deviceFrameLayout->setContentsMargins(12, 10, 12, 10);
    m_deviceFrameLayout->setSpacing(4);

    // Dynamic Island
    m_dynamicIsland = new QFrame(m_deviceFrame);
    m_dynamicIsland->setObjectName("dynamicIsland");
    m_dynamicIsland->setFixedSize(116, 26);
    auto* islandLayout = new QHBoxLayout(m_dynamicIsland);
    islandLayout->setContentsMargins(8, 0, 8, 0);
    auto* cameraDot = new QLabel(m_dynamicIsland);
    cameraDot->setFixedSize(9, 9);
    cameraDot->setStyleSheet("background-color: #0d131f; border-radius: 4px; border: 1px solid #1e293b;");
    islandLayout->addStretch();
    islandLayout->addWidget(cameraDot);
    m_deviceFrameLayout->addWidget(m_dynamicIsland, 0, Qt::AlignHCenter);

    // Screen Content Area (Hosts WebView2 or Placeholder)
    m_contentArea = new QWidget(m_deviceFrame);
    m_contentArea->setObjectName("emulatorContentArea");
    m_contentLayout = new QVBoxLayout(m_contentArea);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);
    m_contentLayout->setSpacing(0);

    m_placeholderLabel = new QLabel(
        "📱 Mobile Viewport Ready\n\nStart a session in the Workbench to launch Expo Preview",
        m_contentArea
    );
    m_placeholderLabel->setAlignment(Qt::AlignCenter);
    m_placeholderLabel->setStyleSheet("color: #64748B; font-size: 13px; font-weight: 500; line-height: 1.5;");
    m_contentLayout->addWidget(m_placeholderLabel);

    m_deviceFrameLayout->addWidget(m_contentArea, 1);

    // Home Indicator
    m_homeIndicator = new QFrame(m_deviceFrame);
    m_homeIndicator->setObjectName("homeIndicator");
    m_homeIndicator->setFixedSize(128, 4);
    m_deviceFrameLayout->addWidget(m_homeIndicator, 0, Qt::AlignHCenter);

    canvasCenteringLayout->addWidget(m_deviceFrame);
    canvasVLayout->addWidget(m_canvasArea, 1);

    // Bottom Subtle Truth In Engineering Bar
    auto* bottomBar = new QWidget(canvasContainer);
    bottomBar->setFixedHeight(26);
    bottomBar->setStyleSheet("background-color: #121418; border-top: 1px solid #2A303C;");
    auto* bottomLayout = new QHBoxLayout(bottomBar);
    bottomLayout->setContentsMargins(12, 0, 12, 0);

    m_truthDisclosureLabel = new QLabel("Expo Web Preview — Not an iOS Runtime", bottomBar);
    m_truthDisclosureLabel->setStyleSheet("color: #F5B84D; font-size: 11px; font-weight: 600;");
    bottomLayout->addWidget(m_truthDisclosureLabel);

    bottomLayout->addStretch();

    m_dimensionStatusLabel = new QLabel("390 x 844 (100%)", bottomBar);
    m_dimensionStatusLabel->setStyleSheet("color: #A4ADBD; font-size: 11px;");
    bottomLayout->addWidget(m_dimensionStatusLabel);

    canvasVLayout->addWidget(bottomBar);
    mainLayout->addWidget(canvasContainer, 1);

    // 2. Adjacent Side Toolbar
    m_sideToolbar = new QWidget(m_centralContainer);
    m_sideToolbar->setObjectName("emulatorToolbar");
    m_sideToolbar->setFixedWidth(46);
    mainLayout->addWidget(m_sideToolbar);
}

void DeviceEmulatorWindow::setupToolbar() {
    auto* tbLayout = new QVBoxLayout(m_sideToolbar);
    tbLayout->setContentsMargins(4, 10, 4, 10);
    tbLayout->setSpacing(6);
    tbLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    auto createToolBtn = [this](const QString& text, const QString& tooltip, bool checkable = false) {
        auto* btn = new QToolButton(m_sideToolbar);
        btn->setText(text);
        btn->setToolTip(tooltip);
        btn->setFixedSize(36, 36);
        btn->setCheckable(checkable);
        btn->setStyleSheet(Theme::iconButtonStyle());
        return btn;
    };

    // Power Button
    m_btnPower = createToolBtn("⏻", "Power: Toggle Screen Sleep / Wake");
    connect(m_btnPower, &QToolButton::clicked, this, &DeviceEmulatorWindow::onPowerClicked);
    tbLayout->addWidget(m_btnPower);

    // Reload Button
    m_btnReload = createToolBtn("🔄", "Reload Preview (Ctrl+R)");
    connect(m_btnReload, &QToolButton::clicked, this, &DeviceEmulatorWindow::onReloadClicked);
    tbLayout->addWidget(m_btnReload);

    // Rotate Button
    m_btnRotate = createToolBtn("↷", "Rotate 90° (Portrait / Landscape)");
    connect(m_btnRotate, &QToolButton::clicked, this, &DeviceEmulatorWindow::onRotateClicked);
    tbLayout->addWidget(m_btnRotate);

    // Device Preset Menu Button
    m_btnPreset = createToolBtn("📱", "Select Device Preset");
    m_presetMenu = new QMenu(m_btnPreset);
    m_presetMenu->setStyleSheet(
        "QMenu { background-color: #1E222A; border: 1px solid #343B48; color: #F2F4F8; padding: 4px; }"
        "QMenu::item { padding: 6px 20px 6px 12px; border-radius: 4px; }"
        "QMenu::item:selected { background-color: #4E9CFF; color: #FFFFFF; }"
    );
    connect(m_presetMenu, &QMenu::triggered, this, &DeviceEmulatorWindow::onPresetMenuTriggered);
    m_btnPreset->setMenu(m_presetMenu);
    m_btnPreset->setPopupMode(QToolButton::InstantPopup);
    tbLayout->addWidget(m_btnPreset);

    // Divider
    auto* div1 = new QFrame(m_sideToolbar);
    div1->setFrameShape(QFrame::HLine);
    div1->setStyleSheet("background-color: #2A303C; max-height: 1px; margin: 4px 2px;");
    tbLayout->addWidget(div1);

    // Screenshot Button
    m_btnScreenshot = createToolBtn("📷", "Take Screenshot & Copy to Clipboard (Ctrl+S)");
    connect(m_btnScreenshot, &QToolButton::clicked, this, &DeviceEmulatorWindow::onScreenshotClicked);
    tbLayout->addWidget(m_btnScreenshot);

    // Touch Emulation Toggle
    m_btnTouch = createToolBtn("👆", "Toggle Touch Emulation", true);
    m_btnTouch->setChecked(m_viewportManager ? m_viewportManager->isTouchEmulationEnabled() : true);
    connect(m_btnTouch, &QToolButton::clicked, this, &DeviceEmulatorWindow::onTouchToggled);
    tbLayout->addWidget(m_btnTouch);

    // Divider
    auto* div2 = new QFrame(m_sideToolbar);
    div2->setFrameShape(QFrame::HLine);
    div2->setStyleSheet("background-color: #2A303C; max-height: 1px; margin: 4px 2px;");
    tbLayout->addWidget(div2);

    // Zoom Controls
    m_btnZoomIn = createToolBtn("+", "Zoom In");
    connect(m_btnZoomIn, &QToolButton::clicked, this, &DeviceEmulatorWindow::onZoomInClicked);
    tbLayout->addWidget(m_btnZoomIn);

    m_btnZoomOut = createToolBtn("−", "Zoom Out");
    connect(m_btnZoomOut, &QToolButton::clicked, this, &DeviceEmulatorWindow::onZoomOutClicked);
    tbLayout->addWidget(m_btnZoomOut);

    m_btnFitWindow = createToolBtn("⛶", "Fit to Window", true);
    m_btnFitWindow->setChecked(m_zoomMode == EmulatorZoomMode::FitWindow);
    connect(m_btnFitWindow, &QToolButton::clicked, this, &DeviceEmulatorWindow::onFitWindowClicked);
    tbLayout->addWidget(m_btnFitWindow);

    m_btnActualSize = createToolBtn("1:1", "Actual Size (100%)", true);
    m_btnActualSize->setChecked(m_zoomMode == EmulatorZoomMode::Fixed && std::abs(m_fixedScale - 1.0) < 0.05);
    connect(m_btnActualSize, &QToolButton::clicked, this, &DeviceEmulatorWindow::onActualSizeClicked);
    tbLayout->addWidget(m_btnActualSize);

    // Divider
    auto* div3 = new QFrame(m_sideToolbar);
    div3->setFrameShape(QFrame::HLine);
    div3->setStyleSheet("background-color: #2A303C; max-height: 1px; margin: 4px 2px;");
    tbLayout->addWidget(div3);

    // Always on Top
    m_btnPin = createToolBtn("📌", "Always on Top (Pin Window)", true);
    m_btnPin->setChecked(m_alwaysOnTop);
    connect(m_btnPin, &QToolButton::clicked, this, &DeviceEmulatorWindow::onPinToggled);
    tbLayout->addWidget(m_btnPin);

    // Bezels / Frame Toggle
    m_btnBezels = createToolBtn("🔲", "Toggle Phone Bezels & Frame", true);
    m_btnBezels->setChecked(m_bezelsVisible);
    connect(m_btnBezels, &QToolButton::clicked, this, &DeviceEmulatorWindow::onBezelsToggled);
    tbLayout->addWidget(m_btnBezels);

    // Open Workbench Console
    m_btnOpenConsole = createToolBtn("💻", "Focus Workbench Console");
    connect(m_btnOpenConsole, &QToolButton::clicked, this, [this]() {
        emit consoleRequested();
    });
    tbLayout->addWidget(m_btnOpenConsole);

    tbLayout->addStretch();

    // Dock / Undock
    m_btnDock = createToolBtn("⇲", "Dock to Workbench");
    connect(m_btnDock, &QToolButton::clicked, this, &DeviceEmulatorWindow::onDockToggled);
    tbLayout->addWidget(m_btnDock);
}

void DeviceEmulatorWindow::setWebView(engines::WebView2Widget* webView) {
    if (m_webView == webView) return;

    if (m_webView && m_webView->parent() == m_contentArea) {
        m_contentLayout->removeWidget(m_webView);
    }

    m_webView = webView;

    if (m_webView) {
        m_contentLayout->addWidget(m_webView);
        m_webView->show();
        if (m_placeholderLabel) {
            m_placeholderLabel->hide();
        }
    } else {
        if (m_placeholderLabel) {
            m_placeholderLabel->show();
        }
    }
}

void DeviceEmulatorWindow::showPlaceholder(bool show, const QString& text) {
    if (m_placeholderLabel) {
        if (!text.isEmpty()) {
            m_placeholderLabel->setText(text);
        }
        m_placeholderLabel->setVisible(show);
    }
    if (m_webView) {
        m_webView->setVisible(!show);
    }
}

void DeviceEmulatorWindow::navigate(const QString& url) {
    if (m_webView) {
        showPlaceholder(false);
        m_webView->navigate(url);
    }
}

void DeviceEmulatorWindow::reload() {
    if (m_webView) {
        m_webView->reload();
    }
    emit reloadRequested();
}

void DeviceEmulatorWindow::setAlwaysOnTop(bool onTop) {
    m_alwaysOnTop = onTop;
    setWindowFlag(Qt::WindowStaysOnTopHint, onTop);
    show();
    if (m_btnPin) {
        m_btnPin->setChecked(onTop);
    }
}

void DeviceEmulatorWindow::setBezelsVisible(bool visible) {
    m_bezelsVisible = visible;
    if (m_btnBezels) {
        m_btnBezels->setChecked(visible);
    }
    updateAppearance();
    recalculateScale();
    updateFrameDimensions();
}

void DeviceEmulatorWindow::setZoomMode(EmulatorZoomMode mode, double fixedScale) {
    m_zoomMode = mode;
    m_fixedScale = std::clamp(fixedScale, 0.25, 3.0);
    if (m_btnFitWindow) {
        m_btnFitWindow->setChecked(m_zoomMode == EmulatorZoomMode::FitWindow);
    }
    if (m_btnActualSize) {
        m_btnActualSize->setChecked(m_zoomMode == EmulatorZoomMode::Fixed && std::abs(m_fixedScale - 1.0) < 0.05);
    }
    recalculateScale();
    updateFrameDimensions();
}

void DeviceEmulatorWindow::setDocked(bool docked) {
    m_isDocked = docked;
    if (m_btnDock) {
        m_btnDock->setText(docked ? "⇱" : "⇲");
        m_btnDock->setToolTip(docked ? "Undock to Independent Window" : "Dock to Workbench");
    }
}

void DeviceEmulatorWindow::updateAppearance() {
    if (!m_deviceFrame) return;

    if (m_bezelsVisible) {
        m_deviceFrame->setStyleSheet(
            "QFrame#emulatorDeviceFrame {"
            "    background-color: #1E222A;"
            "    border: 3px solid #343B48;"
            "    border-radius: 36px;"
            "}"
        );
        m_deviceFrameLayout->setContentsMargins(12, 10, 12, 10);
        
        bool hasDynamicIsland = m_viewportManager && m_viewportManager->activeProfile().hasDynamicIsland;
        m_dynamicIsland->setVisible(hasDynamicIsland && !m_viewportManager->isLandscape());
        m_homeIndicator->setVisible(!m_viewportManager || !m_viewportManager->isLandscape());
    } else {
        m_deviceFrame->setStyleSheet(
            "QFrame#emulatorDeviceFrame {"
            "    background-color: #0F1115;"
            "    border: 1px solid #2A303C;"
            "    border-radius: 4px;"
            "}"
        );
        m_deviceFrameLayout->setContentsMargins(0, 0, 0, 0);
        m_dynamicIsland->hide();
        m_homeIndicator->hide();
    }
}

void DeviceEmulatorWindow::updateWindowHeader() {
    if (!m_viewportManager) return;

    auto profile = m_viewportManager->activeProfile();
    int w = m_viewportManager->currentPixelWidth();
    int h = m_viewportManager->currentPixelHeight();
    int pct = static_cast<int>(std::round(m_currentScale * 100.0));

    setWindowTitle(QString("IOneXa — %1 (%2 x %3)").arg(profile.name).arg(w).arg(h));
    if (m_dimensionStatusLabel) {
        m_dimensionStatusLabel->setText(
            QString("%1 x %2 (%3%) %4")
                .arg(w).arg(h).arg(pct)
                .arg(m_viewportManager->isLandscape() ? "Landscape" : "Portrait")
        );
    }
}

void DeviceEmulatorWindow::recalculateScale() {
    if (!m_viewportManager || !m_canvasArea) return;

    int logW = m_viewportManager->currentPixelWidth();
    int logH = m_viewportManager->currentPixelHeight();
    if (logW <= 0 || logH <= 0) return;

    if (m_zoomMode == EmulatorZoomMode::FitWindow) {
        int availW = m_canvasArea->width() - 48;
        int availH = m_canvasArea->height() - 48;
        if (availW > 100 && availH > 100) {
            double scaleW = static_cast<double>(availW) / logW;
            double scaleH = static_cast<double>(availH) / logH;
            m_currentScale = std::clamp((std::min)(scaleW, scaleH), 0.25, 2.5);
        }
    } else {
        m_currentScale = m_fixedScale;
    }
}

void DeviceEmulatorWindow::updateFrameDimensions() {
    if (!m_viewportManager || !m_deviceFrame) return;

    int logW = m_viewportManager->currentPixelWidth();
    int logH = m_viewportManager->currentPixelHeight();

    int targetW = static_cast<int>(std::round(logW * m_currentScale));
    int targetH = static_cast<int>(std::round(logH * m_currentScale));

    if (m_bezelsVisible) {
        targetW += 24; // padding
        targetH += 24;
    }

    m_deviceFrame->setFixedSize(targetW, targetH);
    updateWindowHeader();
}

void DeviceEmulatorWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    if (m_zoomMode == EmulatorZoomMode::FitWindow) {
        recalculateScale();
        updateFrameDimensions();
    }
}

void DeviceEmulatorWindow::closeEvent(QCloseEvent* event) {
    saveSettings();
    emit emulatorClosed();
    event->accept();
}

void DeviceEmulatorWindow::onViewportChanged() {
    updateAppearance();

    // Rebuild preset menu
    if (m_presetMenu && m_viewportManager) {
        m_presetMenu->clear();
        int activeIdx = m_viewportManager->activeProfileIndex();
        const auto& profiles = m_viewportManager->profiles();
        for (int i = 0; i < profiles.size(); ++i) {
            const auto& p = profiles[i];
            auto* act = m_presetMenu->addAction(QString("%1 (%2x%3)").arg(p.name).arg(p.width).arg(p.height));
            act->setData(i);
            act->setCheckable(true);
            act->setChecked(i == activeIdx);
        }
    }

    recalculateScale();
    updateFrameDimensions();
}

void DeviceEmulatorWindow::onPowerClicked() {
    m_screenPoweredOn = !m_screenPoweredOn;
    if (m_screenPoweredOn) {
        m_contentArea->show();
        m_deviceFrame->setStyleSheet(m_bezelsVisible ?
            "QFrame#emulatorDeviceFrame { background-color: #1E222A; border: 3px solid #343B48; border-radius: 36px; }" :
            "QFrame#emulatorDeviceFrame { background-color: #0F1115; border: 1px solid #2A303C; border-radius: 4px; }"
        );
        if (m_btnPower) m_btnPower->setText("⏻");
    } else {
        m_contentArea->hide();
        m_deviceFrame->setStyleSheet(
            "QFrame#emulatorDeviceFrame { background-color: #000000; border: 3px solid #1E222A; border-radius: 36px; }"
        );
        if (m_btnPower) m_btnPower->setText("⚡");
    }
}

void DeviceEmulatorWindow::onReloadClicked() {
    reload();
}

void DeviceEmulatorWindow::onRotateClicked() {
    if (m_viewportManager) {
        m_viewportManager->toggleOrientation();
    }
}

void DeviceEmulatorWindow::onPresetMenuTriggered(QAction* action) {
    if (!action || !m_viewportManager) return;
    int index = action->data().toInt();
    m_viewportManager->selectProfile(index);
}

void DeviceEmulatorWindow::onZoomInClicked() {
    double newScale = m_currentScale + 0.15;
    setZoomMode(EmulatorZoomMode::Fixed, newScale);
}

void DeviceEmulatorWindow::onZoomOutClicked() {
    double newScale = m_currentScale - 0.15;
    setZoomMode(EmulatorZoomMode::Fixed, newScale);
}

void DeviceEmulatorWindow::onFitWindowClicked() {
    setZoomMode(EmulatorZoomMode::FitWindow);
}

void DeviceEmulatorWindow::onActualSizeClicked() {
    setZoomMode(EmulatorZoomMode::Fixed, 1.0);
}

QString DeviceEmulatorWindow::takeScreenshot(bool copyToClipboard) {
    QPixmap pixmap;
    if (m_contentArea && m_contentArea->isVisible()) {
        pixmap = m_contentArea->grab();
    } else if (m_deviceFrame) {
        pixmap = m_deviceFrame->grab();
    }

    if (pixmap.isNull()) {
        return QString();
    }

    if (copyToClipboard) {
        QApplication::clipboard()->setPixmap(pixmap);
    }

    QString picturesDir = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    if (picturesDir.isEmpty()) {
        picturesDir = QDir::tempPath();
    }
    QString saveFolder = picturesDir + "/IOneXa_Screenshots";
    QDir().mkpath(saveFolder);

    QString fileName = QString("IOneXa_%1_%2.png")
        .arg(m_viewportManager ? m_viewportManager->activeProfile().id : "screen")
        .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));

    QString fullPath = saveFolder + "/" + fileName;
    if (pixmap.save(fullPath, "PNG")) {
        emit screenshotCaptured(fullPath);
        return fullPath;
    }
    return QString();
}

void DeviceEmulatorWindow::onScreenshotClicked() {
    QString savedPath = takeScreenshot(true);
    if (!savedPath.isEmpty()) {
        QToolTip::showText(
            m_btnScreenshot->mapToGlobal(QPoint(0, 0)),
            QString("Screenshot copied to clipboard!\nSaved: %1").arg(savedPath),
            m_btnScreenshot,
            QRect(),
            3000
        );
    }
}

void DeviceEmulatorWindow::onTouchToggled() {
    if (m_viewportManager) {
        bool enable = !m_viewportManager->isTouchEmulationEnabled();
        m_viewportManager->setTouchEmulation(enable);
    }
}

void DeviceEmulatorWindow::onPinToggled() {
    setAlwaysOnTop(!m_alwaysOnTop);
}

void DeviceEmulatorWindow::onBezelsToggled() {
    setBezelsVisible(!m_bezelsVisible);
}

void DeviceEmulatorWindow::onDockToggled() {
    if (m_isDocked) {
        emit undockRequested();
    } else {
        emit dockRequested();
    }
}

void DeviceEmulatorWindow::saveSettings() {
    QSettings settings("UshaLabs", "IOneXa");
    settings.beginGroup("emulator");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("alwaysOnTop", m_alwaysOnTop);
    settings.setValue("bezelsVisible", m_bezelsVisible);
    settings.setValue("zoomMode", m_zoomMode == EmulatorZoomMode::FitWindow ? "fit" : "fixed");
    settings.setValue("fixedScale", m_fixedScale);
    settings.endGroup();
}

void DeviceEmulatorWindow::loadSettings() {
    QSettings settings("UshaLabs", "IOneXa");
    settings.beginGroup("emulator");
    if (settings.contains("geometry")) {
        restoreGeometry(settings.value("geometry").toByteArray());
    } else {
        resize(480, 850);
    }
    m_alwaysOnTop = settings.value("alwaysOnTop", false).toBool();
    if (m_alwaysOnTop) {
        setAlwaysOnTop(true);
    }
    m_bezelsVisible = settings.value("bezelsVisible", true).toBool();
    QString zMode = settings.value("zoomMode", "fit").toString();
    m_fixedScale = settings.value("fixedScale", 1.0).toDouble();
    m_zoomMode = (zMode == "fixed") ? EmulatorZoomMode::Fixed : EmulatorZoomMode::FitWindow;
    settings.endGroup();
}

} // namespace ionexa::ui
