#pragma once

#include <QTest>
#include <QFile>
#include <QFileInfo>
#include <QCryptographicHash>
#include "project/project_scanner.hpp"

#ifndef PROJECT_ROOT_DIR
#define PROJECT_ROOT_DIR "."
#endif

using namespace ionexa::project;

class TestProjectScanner : public QObject {
    Q_OBJECT

private:
    QString getRootDir() const {
        return QString::fromUtf8(PROJECT_ROOT_DIR);
    }

private slots:
    void testBasicExpoDetection() {
        QString fixturePath = getRootDir() + "/tests/fixtures/expo-basic";
        ProjectDetails details = ProjectScanner::scan(fixturePath.toStdString());

        QVERIFY(details.isValid);
        QVERIFY(details.isExpoProject);
        QCOMPARE(QString::fromStdString(details.projectName), QString("expo-basic"));
        QVERIFY(details.hasWebSupport);
        QCOMPARE(QString::fromStdString(details.packageManager), QString("npm"));
        QVERIFY(details.nodeInstalled);
    }

    void testExpoRouterDetection() {
        QString fixturePath = getRootDir() + "/tests/fixtures/expo-router";
        ProjectDetails details = ProjectScanner::scan(fixturePath.toStdString());

        QVERIFY(details.isValid);
        QVERIFY(details.isExpoProject);
        QVERIFY(details.isExpoRouter);
        QVERIFY(details.hasWebSupport);
    }

    void testNonExpoDetection() {
        QString fixturePath = getRootDir() + "/tests/fixtures/non-expo";
        ProjectDetails details = ProjectScanner::scan(fixturePath.toStdString());

        QVERIFY(details.isValid);
        QVERIFY(!details.isExpoProject);
        QCOMPARE(QString::fromStdString(details.projectName), QString("non-expo-app"));
    }

    void testMalformedPackageJson() {
        QString fixturePath = getRootDir() + "/tests/fixtures/malformed-package";
        ProjectDetails details = ProjectScanner::scan(fixturePath.toStdString());

        QVERIFY(!details.isValid);
        QVERIFY(QString::fromStdString(details.errorMessage).contains("Malformed"));
    }

    void testSpacesInPathAndMissingWebDeps() {
        QString fixturePath = getRootDir() + "/tests/fixtures/spaces in path/expo-app";
        ProjectDetails details = ProjectScanner::scan(fixturePath.toStdString());

        QVERIFY(details.isValid);
        QVERIFY(details.isExpoProject);
        QVERIFY(QString::fromStdString(details.canonicalPath).contains("spaces in path"));
        QVERIFY(!details.hasWebSupport);
        QVERIFY(!details.missingWebDependencies.empty());
        QVERIFY(QString::fromStdString(details.remediationCommand).startsWith("npx expo install"));
    }

    void testNonExistentDirectory() {
        ProjectDetails details = ProjectScanner::scan("C:/This/Path/Definitely/Does/Not/Exist_XYZ123");
        QVERIFY(!details.isValid);
        QCOMPARE(QString::fromStdString(details.errorMessage), QString("Directory does not exist"));
    }

    void testZeroMutationIntegrity() {
        QString targetFile = getRootDir() + "/tests/fixtures/expo-basic/package.json";
        QFile f(targetFile);
        QVERIFY(f.open(QIODevice::ReadOnly));
        QByteArray beforeHash = QCryptographicHash::hash(f.readAll(), QCryptographicHash::Sha256);
        f.close();

        // Perform scan
        ProjectScanner::scan((getRootDir() + "/tests/fixtures/expo-basic").toStdString());

        // Verify post-scan hash is 100% identical
        QVERIFY(f.open(QIODevice::ReadOnly));
        QByteArray afterHash = QCryptographicHash::hash(f.readAll(), QCryptographicHash::Sha256);
        f.close();

        QCOMPARE(beforeHash, afterHash);
    }
};
