#pragma once

#include <QTest>
#include <QObject>
#include <QSignalSpy>
#include <QCloseEvent>

#include "ui/device_emulator_window.hpp"
#include "ui/viewport_manager.hpp"

using namespace ionexa::ui;

class TestWindowReliability : public QObject {
    Q_OBJECT

private slots:
    void testWindowCloseGracefulHide();
    void testErrorOverlayDisplayAndDismissal();
    void testFractionalScalingCalculations();
    void testMultiMonitorAspectRatioInvariance();
};

inline void TestWindowReliability::testWindowCloseGracefulHide() {
    ViewportManager vp;
    DeviceEmulatorWindow emulator(&vp);
    emulator.show();
    QVERIFY(!emulator.isHidden());

    QSignalSpy closeSpy(&emulator, &DeviceEmulatorWindow::emulatorClosed);

    // Call close() to trigger closeEvent
    emulator.close();

    // Window must be hidden gracefully without destroying state
    QCOMPARE(closeSpy.count(), 1);
    QVERIFY(emulator.isHidden());
}

inline void TestWindowReliability::testErrorOverlayDisplayAndDismissal() {
    ViewportManager vp;
    DeviceEmulatorWindow emulator(&vp);
    emulator.show();

    emulator.showErrorOverlay(
        "Dev Server Offline",
        "Could not reach 127.0.0.1:8081",
        "Start Metro from Workbench"
    );

    auto* overlay = emulator.findChild<QFrame*>("emulatorErrorOverlay");
    QVERIFY(overlay != nullptr);
    QVERIFY(!overlay->isHidden());

    emulator.hideErrorOverlay();
    QVERIFY(overlay->isHidden());
}

inline void TestWindowReliability::testFractionalScalingCalculations() {
    ViewportManager vp;
    DeviceEmulatorWindow emulator(&vp);

    // Test across standard Windows scaling tiers: 100%, 125%, 150%, 175%, 200%
    const QVector<double> scalingTiers = {1.0, 1.25, 1.5, 1.75, 2.0};
    for (double tier : scalingTiers) {
        emulator.setZoomMode(EmulatorZoomMode::Fixed, tier);
        QCOMPARE(emulator.currentScale(), tier);
        QVERIFY(emulator.minimumWidth() > 0);
        QVERIFY(emulator.minimumHeight() > 0);
    }
}

inline void TestWindowReliability::testMultiMonitorAspectRatioInvariance() {
    ViewportManager vp;
    DeviceEmulatorWindow emulator(&vp);

    int logW = vp.currentPixelWidth();
    int logH = vp.currentPixelHeight();
    double expectedAspect = static_cast<double>(logW) / logH;

    // Simulate resizing to wide monitor dimension
    emulator.resize(1200, 700);
    emulator.setZoomMode(EmulatorZoomMode::FitWindow);
    QVERIFY(emulator.currentScale() > 0.0);

    // Simulate resizing to tall monitor dimension
    emulator.resize(600, 1000);
    emulator.setZoomMode(EmulatorZoomMode::FitWindow);
    QVERIFY(emulator.currentScale() > 0.0);

    // Toggle orientation and recheck
    vp.toggleOrientation();
    double rotatedAspect = static_cast<double>(vp.currentPixelWidth()) / vp.currentPixelHeight();
    QVERIFY(std::abs(rotatedAspect - (1.0 / expectedAspect)) < 0.001);
}
