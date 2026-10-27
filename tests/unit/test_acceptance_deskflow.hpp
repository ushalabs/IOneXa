#pragma once

#include <QTest>
#include <QObject>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>

#include "project/project_scanner.hpp"
#include "project/compatibility_analyzer.hpp"
#include "project/handoff_generator.hpp"
#include "ui/viewport_manager.hpp"
#include "ui/device_emulator_window.hpp"

using namespace ionexa::project;
using namespace ionexa::ui;

class TestAcceptanceDeskflow : public QObject {
    Q_OBJECT

private slots:
    void testDeskflowScannerAcceptance();
    void testDeskflowCompatibilityAcceptance();
    void testDeskflowHandoffGuideGeneration();
    void testDeskflowEmulatorLifecycle();
    void testDeskflowProjectImmutability();
};

inline void TestAcceptanceDeskflow::testDeskflowScannerAcceptance() {
    QString deskflowPath = "C:/Users/HP/Desktop/Projects/DeskFlow";
    if (!QDir(deskflowPath).exists()) {
        QSKIP("DeskFlow project path not found on this system.");
    }

    auto details = ProjectScanner::scan(deskflowPath.toStdString());
    QVERIFY(details.isValid);
    QVERIFY(details.isExpoProject);
    QCOMPARE(QString::fromStdString(details.projectName), QString("deskflow"));
    QVERIFY(QString::fromStdString(details.expoSdkVersion).contains("54"));
    QVERIFY(QString::fromStdString(details.reactNativeVersion).contains("0.81"));
    QCOMPARE(QString::fromStdString(details.entryPoint), QString("expo-router/entry"));
    QCOMPARE(details.isExpoRouter, true);
    QCOMPARE(details.hasWebSupport, true);
}

inline void TestAcceptanceDeskflow::testDeskflowCompatibilityAcceptance() {
    QString deskflowPath = "C:/Users/HP/Desktop/Projects/DeskFlow";
    if (!QDir(deskflowPath).exists()) {
        QSKIP("DeskFlow project path not found on this system.");
    }

    auto report = CompatibilityAnalyzer::analyze(deskflowPath);
    QVERIFY(report.totalModulesAnalyzed > 0);
    QVERIFY(report.compatibilityScore > 50.0);
    QVERIFY(!report.tierSummary().isEmpty());
    QVERIFY(!report.recommendations.isEmpty());
}

inline void TestAcceptanceDeskflow::testDeskflowHandoffGuideGeneration() {
    QString deskflowPath = "C:/Users/HP/Desktop/Projects/DeskFlow";
    if (!QDir(deskflowPath).exists()) {
        QSKIP("DeskFlow project path not found on this system.");
    }

    auto details = ProjectScanner::scan(deskflowPath.toStdString());
    auto report = CompatibilityAnalyzer::analyze(deskflowPath);

    QString markdown = HandoffGenerator::generateMarkdown(details, report);
    QVERIFY(!markdown.isEmpty());
    QVERIFY(markdown.contains("Native iOS Verification Handoff Guide"));
    QVERIFY(markdown.contains("deskflow"));
    QVERIFY(markdown.contains("Expo Go"));
    QVERIFY(markdown.contains("Xcode"));
}

inline void TestAcceptanceDeskflow::testDeskflowEmulatorLifecycle() {
    ViewportManager vp;
    // Calibrate to iPhone 15 Pro Max for full-scale DeskFlow testing
    bool found = vp.selectProfileById("iphone-15-pro-max");
    QVERIFY(found);

    DeviceEmulatorWindow emulator(&vp);
    emulator.resize(500, 850);
    emulator.show();
    QTest::qWait(50);

    QVERIFY(emulator.windowTitle().contains("iPhone 15 Pro Max"));
    QCOMPARE(vp.currentPixelWidth(), 430);
    QCOMPARE(vp.currentPixelHeight(), 932);

    // Test zoom and screenshot
    emulator.setZoomMode(EmulatorZoomMode::Fixed, 1.0);
    QString screenshotPath = emulator.takeScreenshot(false);
    QVERIFY(!screenshotPath.isEmpty());
    QVERIFY(QFile::exists(screenshotPath));
    QFile::remove(screenshotPath);

    emulator.close();
}

inline void TestAcceptanceDeskflow::testDeskflowProjectImmutability() {
    QString deskflowPath = "C:/Users/HP/Desktop/Projects/DeskFlow";
    if (!QDir(deskflowPath).exists()) {
        QSKIP("DeskFlow project path not found on this system.");
    }

    // Verify package.json exists and was NOT corrupted or altered
    QFile pkgFile(deskflowPath + "/package.json");
    QVERIFY(pkgFile.exists());
    QVERIFY(pkgFile.open(QIODevice::ReadOnly));
    QByteArray data = pkgFile.readAll();
    pkgFile.close();

    QVERIFY(data.contains("\"name\": \"deskflow\""));
    QVERIFY(data.contains("\"expo\": \"54.0.13\""));
}
