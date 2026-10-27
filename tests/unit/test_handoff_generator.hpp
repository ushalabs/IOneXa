#pragma once

#include <QTest>
#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "project/handoff_generator.hpp"
#include "project/compatibility_analyzer.hpp"
#include "project/project_scanner.hpp"

using namespace ionexa::project;

class TestHandoffGenerator : public QObject {
    Q_OBJECT

private slots:
    void testHandoffPlanGeneration();
    void testUnsupportedModuleChecklistFlagging();
    void testPlatformSpecificFilesDetection();
    void testMarkdownExportFidelity();
    void testJsonExportSchema();
};

inline void TestHandoffGenerator::testHandoffPlanGeneration() {
    ProjectDetails project;
    project.projectName = "WorkbenchDemo";
    project.expoSdkVersion = "52.0.0";

    CompatibilityReport report;
    report.projectName = "WorkbenchDemo";

    HandoffPlan plan = HandoffGenerator::generate(project, report);
    QCOMPARE(plan.projectName, QString("WorkbenchDemo"));
    QCOMPARE(plan.expoSdkVersion, QString("52.0.0"));
    QVERIFY(plan.expoGoCommand.contains("npx expo start"));
    QVERIFY(plan.xcodeCommand.contains("run:ios"));
    QVERIFY(plan.easBuildCommand.contains("eas build"));

    // Verify baseline checklist
    QVERIFY(plan.checklist.size() >= 3);
    bool foundSafeArea = false;
    for (const auto& item : plan.checklist) {
        if (item.category == "Viewport & Safe Area") {
            foundSafeArea = true;
            break;
        }
    }
    QVERIFY(foundSafeArea);
}

inline void TestHandoffGenerator::testUnsupportedModuleChecklistFlagging() {
    ProjectDetails project;
    project.projectName = "HardwareApp";

    CompatibilityReport report;
    report.projectName = "HardwareApp";

    ModuleCompatibilityInfo mod;
    mod.moduleName = "react-native-bluetooth";
    mod.tier = CompatibilityTier::Unsupported;
    mod.nativeIosNotes = "Requires iOS CoreBluetooth framework";
    report.moduleResults.push_back(mod);

    HandoffPlan plan = HandoffGenerator::generate(project, report);

    bool foundBlocking = false;
    for (const auto& item : plan.checklist) {
        if (item.moduleName == "react-native-bluetooth") {
            QVERIFY(item.isBlocking);
            QCOMPARE(item.category, QString("Native Hardware & Frameworks"));
            foundBlocking = true;
        }
    }
    QVERIFY(foundBlocking);
}

inline void TestHandoffGenerator::testPlatformSpecificFilesDetection() {
    ProjectDetails project;
    project.projectName = "PlatformApp";

    CompatibilityReport report;
    report.fileAnalysis.iosSpecificFiles = 2;
    report.fileAnalysis.iosFileList = {"app/modal.ios.tsx", "utils/biometrics.ios.ts"};

    HandoffPlan plan = HandoffGenerator::generate(project, report);
    QCOMPARE(plan.iosSpecificFiles.size(), 2);

    bool foundPlatformCheck = false;
    for (const auto& item : plan.checklist) {
        if (item.category == "Platform Code Verification") {
            QVERIFY(item.isBlocking);
            foundPlatformCheck = true;
        }
    }
    QVERIFY(foundPlatformCheck);
}

inline void TestHandoffGenerator::testMarkdownExportFidelity() {
    ProjectDetails project;
    project.projectName = "MarkdownTestApp";
    project.expoSdkVersion = "52.0.0";

    CompatibilityReport report;
    report.recommendations.push_back("Use EAS Build for Apple Developer certificate signing");

    QString md = HandoffGenerator::generateMarkdown(project, report);
    QVERIFY(!md.isEmpty());
    QVERIFY(md.contains("# Native iOS Verification Handoff Guide: MarkdownTestApp"));
    QVERIFY(md.contains("Truth in Engineering Disclosure"));
    QVERIFY(md.contains("Expo Go"));
    QVERIFY(md.contains("Xcode Simulator"));
    QVERIFY(md.contains("Pre-Flight Native Verification Checklist"));
    QVERIFY(md.contains("- [ ]"));
    QVERIFY(md.contains("Use EAS Build"));
}

inline void TestHandoffGenerator::testJsonExportSchema() {
    ProjectDetails project;
    project.projectName = "JsonTestApp";
    project.expoSdkVersion = "51.0.0";

    CompatibilityReport report;
    QString jsonStr = HandoffGenerator::generateJson(project, report);

    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
    QVERIFY(doc.isObject());
    QJsonObject obj = doc.object();
    QCOMPARE(obj.value("projectName").toString(), QString("JsonTestApp"));
    QCOMPARE(obj.value("expoSdkVersion").toString(), QString("51.0.0"));
    QVERIFY(obj.value("checklist").isArray());
    QVERIFY(obj.value("checklist").toArray().size() >= 3);
}
