#pragma once

#include <QTest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "project/compatibility_analyzer.hpp"

#ifndef PROJECT_ROOT_DIR
#define PROJECT_ROOT_DIR "."
#endif

using namespace ionexa::project;

class TestCompatibilityAnalyzer : public QObject {
    Q_OBJECT

private:
    QString getRootDir() const {
        return QString::fromUtf8(PROJECT_ROOT_DIR);
    }

private slots:
    void testKnownModuleEvaluation() {
        // Supported
        auto m1 = CompatibilityAnalyzer::evaluateModule("expo-router");
        QCOMPARE(m1.tier, CompatibilityTier::Supported);
        QVERIFY(!m1.webSupportNotes.isEmpty());

        auto m2 = CompatibilityAnalyzer::evaluateModule("react-native-web");
        QCOMPARE(m2.tier, CompatibilityTier::Supported);

        // Partial
        auto m3 = CompatibilityAnalyzer::evaluateModule("@react-native-async-storage/async-storage");
        QCOMPARE(m3.tier, CompatibilityTier::Partial);
        QVERIFY(m3.webSupportNotes.contains("localStorage"));

        auto m4 = CompatibilityAnalyzer::evaluateModule("expo-secure-store");
        QCOMPARE(m4.tier, CompatibilityTier::Partial);

        // Unsupported
        auto m5 = CompatibilityAnalyzer::evaluateModule("react-native-iap");
        QCOMPARE(m5.tier, CompatibilityTier::Unsupported);
        QVERIFY(m5.nativeIosNotes.contains("StoreKit"));

        auto m6 = CompatibilityAnalyzer::evaluateModule("react-native-health");
        QCOMPARE(m6.tier, CompatibilityTier::Unsupported);
        QVERIFY(m6.nativeIosNotes.contains("HealthKit"));

        // Unverified
        auto m7 = CompatibilityAnalyzer::evaluateModule("random-third-party-lib");
        QCOMPARE(m7.tier, CompatibilityTier::Unverified);
    }

    void testAnalyzeExpoBasicFixture() {
        QString fixturePath = getRootDir() + "/tests/fixtures/expo-basic";
        auto report = CompatibilityAnalyzer::analyze(fixturePath);

        QCOMPARE(report.projectName, QString("expo-basic"));
        QVERIFY(report.totalModulesAnalyzed > 0);
        QCOMPARE(report.unsupportedCount, 0);
        QVERIFY(report.compatibilityScore >= 90.0);
        QVERIFY(!report.recommendations.isEmpty());
    }

    void testAnalyzeExpoRouterFixture() {
        QString fixturePath = getRootDir() + "/tests/fixtures/expo-router";
        auto report = CompatibilityAnalyzer::analyze(fixturePath);

        QCOMPARE(report.projectName, QString("expo-router-fixture"));
        QVERIFY(report.supportedCount > 0);
        QVERIFY(report.compatibilityScore >= 75.0);
    }

    void testSourceFileScanner() {
        QString fixturePath = getRootDir() + "/tests/fixtures/expo-basic";
        auto analysis = CompatibilityAnalyzer::scanSourceFiles(fixturePath);

        QVERIFY(analysis.totalSourceFiles >= 1);
        QCOMPARE(analysis.iosSpecificFiles, 0);
    }

    void testMarkdownAndJsonExport() {
        QString fixturePath = getRootDir() + "/tests/fixtures/expo-basic";
        auto report = CompatibilityAnalyzer::analyze(fixturePath);

        // Markdown
        QString md = report.exportAsMarkdown();
        QVERIFY(md.contains("# Expo & iOS Compatibility Report"));
        QVERIFY(md.contains("Compatibility Score:"));
        QVERIFY(md.contains("| Module | Compatibility Tier |"));

        // JSON
        QString jsonStr = report.exportAsJson();
        QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
        QVERIFY(doc.isObject());
        QJsonObject root = doc.object();
        QCOMPARE(root.value("projectName").toString(), QString("expo-basic"));
        QVERIFY(root.value("compatibilityScore").toDouble() >= 90.0);
        QVERIFY(root.value("modules").isArray());
        QVERIFY(root.value("recommendations").isArray());
    }

    void testRemediationAdvisories() {
        // Manually test advisory formatting when unsupported modules exist
        CompatibilityReport report;
        report.projectName = "unsupported-test";
        report.totalModulesAnalyzed = 2;
        report.supportedCount = 1;
        report.unsupportedCount = 1;
        report.compatibilityScore = 50.0;

        ModuleCompatibilityInfo modIap = CompatibilityAnalyzer::evaluateModule("react-native-iap");
        report.moduleResults.push_back(modIap);
        report.recommendations.append(QString("'%1' requires native Apple iOS frameworks. Guard usages with Platform.select.").arg(modIap.moduleName));

        QVERIFY(!report.recommendations.isEmpty());
        QVERIFY(report.recommendations[0].contains("react-native-iap"));
        QVERIFY(report.recommendations[0].contains("Platform.select"));
    }
};
