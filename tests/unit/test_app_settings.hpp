#pragma once

#include <QTest>
#include <QTemporaryDir>
#include "core/app_settings.hpp"

using namespace ionexa::core;

class TestAppSettings : public QObject {
    Q_OBJECT

private slots:
    void testDefaults() {
        AppSettings settings;
        QCOMPARE(QString::fromStdString(settings.defaultDevicePreset()), QString("iPhone 14/15 (390 x 844)"));
        QCOMPARE(settings.defaultPort(), 8081);
        QVERIFY(!settings.startInLandscape());
        QVERIFY(!settings.telemetryOptIn());
        QVERIFY(!settings.experimentalNativeWindowsRuntime());
        QVERIFY(settings.recentProjects().empty());
    }

    void testRecentProjectsOrder() {
        AppSettings settings;
        settings.addRecentProject("C:/Projects/AppA");
        settings.addRecentProject("C:/Projects/AppB");
        settings.addRecentProject("C:/Projects/AppA"); // Should move to front

        const auto& recents = settings.recentProjects();
        QCOMPARE(recents.size(), static_cast<size_t>(2));
        QCOMPARE(QString::fromStdString(recents[0]), QString("C:/Projects/AppA"));
        QCOMPARE(QString::fromStdString(recents[1]), QString("C:/Projects/AppB"));
    }

    void testJsonSerializationRoundtrip() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        QString filePath = tempDir.path() + "/test_settings.json";

        AppSettings s1;
        s1.setDefaultDevicePreset("Pixel 7 (412 x 915)");
        s1.setDefaultPort(8090);
        s1.setStartInLandscape(true);
        s1.setExperimentalNativeWindowsRuntime(true);
        s1.addRecentProject("C:/Apps/ExpoTest");

        QVERIFY(s1.saveToFile(filePath.toStdString()));

        AppSettings s2;
        QVERIFY(s2.loadFromFile(filePath.toStdString()));
        QCOMPARE(QString::fromStdString(s2.defaultDevicePreset()), QString("Pixel 7 (412 x 915)"));
        QCOMPARE(s2.defaultPort(), 8090);
        QVERIFY(s2.startInLandscape());
        QVERIFY(s2.experimentalNativeWindowsRuntime());
        QCOMPARE(s2.recentProjects().size(), static_cast<size_t>(1));
        QCOMPARE(QString::fromStdString(s2.recentProjects()[0]), QString("C:/Apps/ExpoTest"));
    }
};
