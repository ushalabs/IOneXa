#pragma once

#include <QTest>
#include <QSignalSpy>
#include <QJsonDocument>
#include <QJsonObject>
#include "ui/viewport_manager.hpp"
#include "core/app_settings.hpp"

using namespace ionexa::ui;
using namespace ionexa::core;

class TestViewportPresets : public QObject {
    Q_OBJECT

private slots:
    void testProfileCatalog() {
        ViewportManager vm;
        const auto& profiles = vm.profiles();
        QVERIFY(profiles.size() >= 7);

        auto p0 = vm.activeProfile();
        QCOMPARE(p0.id, QString("iphone-14"));
        QCOMPARE(p0.width, 390);
        QCOMPARE(p0.height, 844);
        QCOMPARE(p0.hasDynamicIsland, true);
        QCOMPARE(p0.scaleFactor, 3.0);
    }

    void testOrientationToggling() {
        ViewportManager vm;
        QCOMPARE(vm.isLandscape(), false);
        QCOMPARE(vm.currentPixelWidth(), 390);
        QCOMPARE(vm.currentPixelHeight(), 844);

        QSignalSpy spy(&vm, &ViewportManager::viewportChanged);
        vm.toggleOrientation();

        QCOMPARE(vm.isLandscape(), true);
        QCOMPARE(vm.currentPixelWidth(), 844);
        QCOMPARE(vm.currentPixelHeight(), 390);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().at(0).toInt(), 844);
        QCOMPARE(spy.first().at(1).toInt(), 390);
        QCOMPARE(spy.first().at(2).toBool(), true);
    }

    void testProfileSelection() {
        ViewportManager vm;
        QSignalSpy profileSpy(&vm, &ViewportManager::profileChanged);
        QSignalSpy viewSpy(&vm, &ViewportManager::viewportChanged);

        bool found = vm.selectProfileById("pixel-7");
        QVERIFY(found);
        QCOMPARE(vm.activeProfile().id, QString("pixel-7"));
        QCOMPARE(vm.currentPixelWidth(), 412);
        QCOMPARE(vm.currentPixelHeight(), 915);
        QCOMPARE(profileSpy.count(), 1);
        QCOMPARE(viewSpy.count(), 1);
    }

    void testCustomDimensionsAndClamping() {
        ViewportManager vm;
        // Test clamping to minimum 320x480
        vm.setCustomDimensions(200, 200);
        QCOMPARE(vm.activeProfile().id, QString("custom"));
        QCOMPARE(vm.currentPixelWidth(), 320);
        QCOMPARE(vm.currentPixelHeight(), 480);

        // Test normal custom dimensions
        vm.setCustomDimensions(480, 800);
        QCOMPARE(vm.currentPixelWidth(), 480);
        QCOMPARE(vm.currentPixelHeight(), 800);
    }

    void testTouchEmulationToggle() {
        ViewportManager vm;
        QCOMPARE(vm.isTouchEmulationEnabled(), true);

        QSignalSpy spy(&vm, &ViewportManager::touchEmulationToggled);
        vm.setTouchEmulation(false);
        QCOMPARE(vm.isTouchEmulationEnabled(), false);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().at(0).toBool(), false);
    }

    void testCdpJsonPayloads() {
        ViewportManager vm;
        vm.selectProfileById("iphone-15-pro-max");

        // Metrics JSON
        QString metricsStr = vm.buildDeviceMetricsJson();
        QJsonDocument doc = QJsonDocument::fromJson(metricsStr.toUtf8());
        QVERIFY(doc.isObject());
        QCOMPARE(doc.object().value("width").toInt(), 430);
        QCOMPARE(doc.object().value("height").toInt(), 932);
        QCOMPARE(doc.object().value("mobile").toBool(), true);

        // Touch JSON
        QString touchStr = vm.buildTouchEmulationJson();
        QJsonDocument touchDoc = QJsonDocument::fromJson(touchStr.toUtf8());
        QVERIFY(touchDoc.isObject());
        QCOMPARE(touchDoc.object().value("enabled").toBool(), true);
        QCOMPARE(touchDoc.object().value("maxTouchPoints").toInt(), 5);

        // Mouse touch JSON
        QString mouseTouchStr = vm.buildEmitTouchForMouseJson();
        QJsonDocument mouseDoc = QJsonDocument::fromJson(mouseTouchStr.toUtf8());
        QVERIFY(mouseDoc.isObject());
        QCOMPARE(mouseDoc.object().value("configuration").toString(), QString("mobile"));

        // User Agent JSON
        QString uaStr = vm.buildUserAgentJson();
        QJsonDocument uaDoc = QJsonDocument::fromJson(uaStr.toUtf8());
        QVERIFY(uaDoc.isObject());
        QVERIFY(uaDoc.object().value("userAgent").toString().contains("iPhone"));
    }

    void testAppSettingsPersistence() {
        AppSettings settings;
        settings.setCustomViewportWidth(480);
        settings.setCustomViewportHeight(960);
        settings.setTouchEmulationEnabled(false);
        settings.setZoomFactor(1.25);

        std::string json = settings.toJson();

        AppSettings loaded;
        QVERIFY(loaded.fromJson(json));
        QCOMPARE(loaded.customViewportWidth(), 480);
        QCOMPARE(loaded.customViewportHeight(), 960);
        QCOMPARE(loaded.touchEmulationEnabled(), false);
        QCOMPARE(loaded.zoomFactor(), 1.25);
    }
};
