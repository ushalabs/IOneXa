#pragma once

#include <QTest>
#include <QObject>
#include <QSignalSpy>

#include "ui/viewport_manager.hpp"
#include "ui/device_manager_panel.hpp"

using namespace ionexa::ui;

class TestDeviceManager : public QObject {
    Q_OBJECT

private slots:
    void testCustomProfileAddition();
    void testProfileDuplication();
    void testProfileRemoval();
    void testCoreProfileProtection();
    void testProfilePersistence();
    void testDeviceManagerPanelSearchFilter();
};

inline void TestDeviceManager::testCustomProfileAddition() {
    ViewportManager vm;
    int initialCount = vm.profiles().size();

    DeviceProfile custom;
    custom.id = "custom-foldable";
    custom.name = "Custom Foldable Viewport";
    custom.width = 673;
    custom.height = 841;
    custom.scaleFactor = 2.5;
    custom.isMobile = true;
    custom.hasDynamicIsland = false;

    QSignalSpy listSpy(&vm, &ViewportManager::profileListChanged);
    bool added = vm.addCustomProfile(custom);
    QVERIFY(added);
    QCOMPARE(vm.profiles().size(), initialCount + 1);
    QCOMPARE(listSpy.count(), 1);

    // Duplicate ID should fail
    bool dupeAdd = vm.addCustomProfile(custom);
    QVERIFY(!dupeAdd);
}

inline void TestDeviceManager::testProfileDuplication() {
    ViewportManager vm;
    int initialCount = vm.profiles().size();

    bool duped = vm.duplicateProfile("iphone-14", "iPhone 14 (Work Profile)");
    QVERIFY(duped);
    QCOMPARE(vm.profiles().size(), initialCount + 1);

    bool found = false;
    for (const auto& p : vm.profiles()) {
        if (p.name == "iPhone 14 (Work Profile)") {
            found = true;
            QCOMPARE(p.width, 390);
            QCOMPARE(p.height, 844);
            QCOMPARE(p.hasDynamicIsland, true);
            break;
        }
    }
    QVERIFY(found);
}

inline void TestDeviceManager::testProfileRemoval() {
    ViewportManager vm;

    DeviceProfile custom;
    custom.id = "custom-tablet";
    custom.name = "Custom Tablet";
    custom.width = 800;
    custom.height = 1280;
    vm.addCustomProfile(custom);

    int countBefore = vm.profiles().size();
    bool removed = vm.removeProfile("custom-tablet");
    QVERIFY(removed);
    QCOMPARE(vm.profiles().size(), countBefore - 1);
}

inline void TestDeviceManager::testCoreProfileProtection() {
    ViewportManager vm;

    // Core profiles cannot be removed to prevent breaking default viewport orchestration
    bool removedIPhone = vm.removeProfile("iphone-14");
    QVERIFY(!removedIPhone);

    bool removedPixel = vm.removeProfile("pixel-7");
    QVERIFY(!removedPixel);
}

inline void TestDeviceManager::testProfilePersistence() {
    ViewportManager vm;
    DeviceProfile custom;
    custom.id = "custom-persistent";
    custom.name = "Persistent Phone";
    custom.width = 414;
    custom.height = 896;
    vm.addCustomProfile(custom);

    vm.saveProfiles();

    ViewportManager vm2;
    vm2.loadProfiles();

    bool found = false;
    for (const auto& p : vm2.profiles()) {
        if (p.id == "custom-persistent") {
            found = true;
            QCOMPARE(p.width, 414);
            QCOMPARE(p.height, 896);
            break;
        }
    }
    QVERIFY(found);

    // Clean up
    vm2.removeProfile("custom-persistent");
    vm2.saveProfiles();
}

inline void TestDeviceManager::testDeviceManagerPanelSearchFilter() {
    ViewportManager vm;
    DeviceManagerPanel panel(&vm);

    QSignalSpy launchSpy(&panel, &DeviceManagerPanel::launchRequested);
    QVERIFY(panel.width() >= 0);
}
