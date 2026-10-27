#pragma once

#include <QTest>
#include <QObject>
#include <QFile>
#include <QFileInfo>
#include <QSignalSpy>

#include "ui/device_emulator_window.hpp"
#include "ui/viewport_manager.hpp"

using namespace ionexa::ui;

class TestEmulatorWindow : public QObject {
    Q_OBJECT

private slots:
    void testEmulatorInitialization();
    void testOrientationChange();
    void testProfileSwitching();
    void testZoomAndScalingModes();
    void testAlwaysOnTopToggle();
    void testBezelsToggle();
    void testDockingState();
    void testScreenshotCapture();
    void testSettingsPersistence();
};

inline void TestEmulatorWindow::testEmulatorInitialization() {
    ViewportManager vp;
    DeviceEmulatorWindow emulator(&vp);

    QVERIFY(emulator.windowTitle().contains("IOneXa"));
    QVERIFY(emulator.windowTitle().contains("iPhone 14 / 15"));
    QCOMPARE(emulator.isDocked(), false);
    QCOMPARE(emulator.areBezelsVisible(), true);
    QVERIFY(emulator.viewportManager() != nullptr);
    QVERIFY(emulator.minimumWidth() >= 400);
}

inline void TestEmulatorWindow::testOrientationChange() {
    ViewportManager vp;
    DeviceEmulatorWindow emulator(&vp);

    QCOMPARE(vp.isLandscape(), false);
    int initW = vp.currentPixelWidth();
    int initH = vp.currentPixelHeight();

    vp.toggleOrientation();
    QCOMPARE(vp.isLandscape(), true);
    QCOMPARE(vp.currentPixelWidth(), initH);
    QCOMPARE(vp.currentPixelHeight(), initW);

    QVERIFY(emulator.windowTitle().contains(QString::number(initH)));
    QVERIFY(emulator.windowTitle().contains(QString::number(initW)));
}

inline void TestEmulatorWindow::testProfileSwitching() {
    ViewportManager vp;
    DeviceEmulatorWindow emulator(&vp);

    bool found = vp.selectProfileById("pixel-7");
    QVERIFY(found);
    QVERIFY(emulator.windowTitle().contains("Google Pixel 7"));
    QCOMPARE(vp.currentPixelWidth(), 412);
    QCOMPARE(vp.currentPixelHeight(), 915);
}

inline void TestEmulatorWindow::testZoomAndScalingModes() {
    ViewportManager vp;
    DeviceEmulatorWindow emulator(&vp);

    emulator.setZoomMode(EmulatorZoomMode::Fixed, 1.0);
    QCOMPARE(emulator.zoomMode(), EmulatorZoomMode::Fixed);
    QCOMPARE(emulator.currentScale(), 1.0);

    emulator.setZoomMode(EmulatorZoomMode::Fixed, 1.5);
    QCOMPARE(emulator.currentScale(), 1.5);

    emulator.setZoomMode(EmulatorZoomMode::FitWindow);
    QCOMPARE(emulator.zoomMode(), EmulatorZoomMode::FitWindow);
}

inline void TestEmulatorWindow::testAlwaysOnTopToggle() {
    ViewportManager vp;
    DeviceEmulatorWindow emulator(&vp);

    emulator.setAlwaysOnTop(true);
    QCOMPARE(emulator.isAlwaysOnTop(), true);
    QVERIFY(emulator.windowFlags().testFlag(Qt::WindowStaysOnTopHint));

    emulator.setAlwaysOnTop(false);
    QCOMPARE(emulator.isAlwaysOnTop(), false);
    QVERIFY(!emulator.windowFlags().testFlag(Qt::WindowStaysOnTopHint));
}

inline void TestEmulatorWindow::testBezelsToggle() {
    ViewportManager vp;
    DeviceEmulatorWindow emulator(&vp);

    emulator.setBezelsVisible(false);
    QCOMPARE(emulator.areBezelsVisible(), false);

    emulator.setBezelsVisible(true);
    QCOMPARE(emulator.areBezelsVisible(), true);
}

inline void TestEmulatorWindow::testDockingState() {
    ViewportManager vp;
    DeviceEmulatorWindow emulator(&vp);

    QSignalSpy dockSpy(&emulator, &DeviceEmulatorWindow::dockRequested);
    QSignalSpy undockSpy(&emulator, &DeviceEmulatorWindow::undockRequested);

    emulator.setDocked(false);
    QCOMPARE(emulator.isDocked(), false);

    emulator.setDocked(true);
    QCOMPARE(emulator.isDocked(), true);
}

inline void TestEmulatorWindow::testScreenshotCapture() {
    ViewportManager vp;
    DeviceEmulatorWindow emulator(&vp);
    emulator.resize(500, 800);
    emulator.show();

    QSignalSpy spy(&emulator, &DeviceEmulatorWindow::screenshotCaptured);
    QString filePath = emulator.takeScreenshot(false);

    QVERIFY(!filePath.isEmpty());
    QVERIFY(QFile::exists(filePath));
    QFileInfo fi(filePath);
    QVERIFY(fi.size() > 0);
    QCOMPARE(fi.suffix().toLower(), QString("png"));

    QCOMPARE(spy.count(), 1);

    // Clean up temporary screenshot
    QFile::remove(filePath);
}

inline void TestEmulatorWindow::testSettingsPersistence() {
    ViewportManager vp;
    {
        DeviceEmulatorWindow emulator(&vp);
        emulator.setAlwaysOnTop(true);
        emulator.setBezelsVisible(false);
        emulator.setZoomMode(EmulatorZoomMode::Fixed, 1.25);
        emulator.saveSettings();
    }

    {
        DeviceEmulatorWindow emulator2(&vp);
        emulator2.loadSettings();
        QCOMPARE(emulator2.isAlwaysOnTop(), true);
        QCOMPARE(emulator2.areBezelsVisible(), false);
        QCOMPARE(emulator2.zoomMode(), EmulatorZoomMode::Fixed);
        QCOMPARE(emulator2.currentScale(), 1.25);

        // Reset to defaults
        emulator2.setAlwaysOnTop(false);
        emulator2.setBezelsVisible(true);
        emulator2.setZoomMode(EmulatorZoomMode::FitWindow);
        emulator2.saveSettings();
    }
}
