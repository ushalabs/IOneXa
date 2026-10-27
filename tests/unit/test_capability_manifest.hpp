#pragma once

#include <QTest>
#include "core/capability_manifest.hpp"

using namespace ionexa::core;

class TestCapabilityManifest : public QObject {
    Q_OBJECT

private slots:
    void testManifestCounts() {
        CapabilityManifest manifest;
        manifest.addItem({"React Native Web Rendering", CapabilityStatus::Supported, "Tested via Metro", ""});
        manifest.addItem({"Expo Router Navigation", CapabilityStatus::Partial, "Web browser history", "Deep link parity partial"});
        manifest.addItem({"Apple Pay / StoreKit", CapabilityStatus::Unsupported, "iOS Native Only", "Requires physical device"});
        manifest.addItem({"Push Notifications (APNs)", CapabilityStatus::Unverified, "Requires genuine iOS", ""});

        QCOMPARE(manifest.items().size(), static_cast<size_t>(4));
        QCOMPARE(manifest.countWithStatus(CapabilityStatus::Supported), static_cast<size_t>(1));
        QCOMPARE(manifest.countWithStatus(CapabilityStatus::Partial), static_cast<size_t>(1));
        QCOMPARE(manifest.countWithStatus(CapabilityStatus::Unsupported), static_cast<size_t>(1));
        QCOMPARE(manifest.countWithStatus(CapabilityStatus::Unverified), static_cast<size_t>(1));
    }

    void testItemAttributes() {
        CapabilityItem item{"Keychain Storage", CapabilityStatus::Unsupported, "Web LocalStorage Shim", "Non-secure fallback"};
        QCOMPARE(QString::fromStdString(item.feature), QString("Keychain Storage"));
        QCOMPARE(QString::fromStdString(capabilityStatusToString(item.status)), QString("unsupported"));
        QCOMPARE(QString::fromStdString(item.evidence), QString("Web LocalStorage Shim"));
    }
};
