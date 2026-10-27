#pragma once

#include <QTest>
#include <QSignalSpy>
#include "engines/fast_refresh_bridge.hpp"

using namespace ionexa::engines;

class TestFastRefresh : public QObject {
    Q_OBJECT

private slots:
    void testInitialState() {
        FastRefreshBridge bridge;
        QCOMPARE(bridge.state(), BridgeState::Idle);
        QCOMPARE(bridge.stats().totalReloads, 0);
        QCOMPARE(bridge.stats().fastRefreshCount, 0);
        QCOMPARE(bridge.stats().bundleErrors, 0);
        QCOMPARE(bridge.isAutoReconnectEnabled(), true);
    }

    void testMetroLogParsingFastRefresh() {
        FastRefreshBridge bridge;
        QSignalSpy refreshSpy(&bridge, &FastRefreshBridge::fastRefreshDetected);
        QSignalSpy statsSpy(&bridge, &FastRefreshBridge::statsUpdated);

        bridge.processMetroLog("[Fast Refresh] done in 42ms");
        QCOMPARE(refreshSpy.count(), 1);
        QCOMPARE(statsSpy.count(), 1);
        QCOMPARE(bridge.stats().fastRefreshCount, 1);
        QVERIFY(bridge.stats().lastRefreshTime.isValid());
    }

    void testMetroLogParsingBundleComplete() {
        FastRefreshBridge bridge;
        QSignalSpy bundleSpy(&bridge, &FastRefreshBridge::bundleCompleted);

        bridge.processMetroLog("Web Bundled 1450ms (index.js)");
        QCOMPARE(bundleSpy.count(), 1);
        QCOMPARE(bundleSpy.first().at(0).toInt(), 1450);
        QCOMPARE(bridge.stats().fastRefreshCount, 1);
    }

    void testMetroLogParsingBundleError() {
        FastRefreshBridge bridge;
        QSignalSpy errorSpy(&bridge, &FastRefreshBridge::bundleError);

        bridge.processMetroLog("SyntaxError: Unexpected token (12:4)");
        QCOMPARE(errorSpy.count(), 1);
        QCOMPARE(bridge.stats().bundleErrors, 1);
        QVERIFY(bridge.stats().lastErrorMessage.contains("SyntaxError"));
    }

    void testManualReloadDispatch() {
        FastRefreshBridge bridge;
        QSignalSpy reloadSpy(&bridge, &FastRefreshBridge::reloadRequested);

        bridge.requestManualReload(false);
        QCOMPARE(reloadSpy.count(), 1);
        QCOMPARE(reloadSpy.first().at(0).toBool(), false);
        QCOMPARE(bridge.stats().totalReloads, 1);

        bridge.requestManualReload(true);
        QCOMPARE(reloadSpy.count(), 2);
        QCOMPARE(reloadSpy.last().at(0).toBool(), true);
        QCOMPARE(bridge.stats().totalReloads, 2);
    }

    void testWebMessageIpc() {
        FastRefreshBridge bridge;
        QSignalSpy refreshSpy(&bridge, &FastRefreshBridge::fastRefreshDetected);
        QSignalSpy errorSpy(&bridge, &FastRefreshBridge::bundleError);

        // Fast refresh message from browser
        bridge.processWebMessage(R"({"type":"fast_refresh_applied"})");
        QCOMPARE(refreshSpy.count(), 1);
        QCOMPARE(bridge.stats().fastRefreshCount, 1);

        // Runtime error message from browser
        bridge.processWebMessage(R"({"type":"runtime_error","message":"TypeError: undefined is not a function"})");
        QCOMPARE(errorSpy.count(), 1);
        QCOMPARE(bridge.stats().bundleErrors, 1);
        QCOMPARE(bridge.stats().lastErrorMessage, QString("TypeError: undefined is not a function"));
    }

    void testDevServerLifecycle() {
        FastRefreshBridge bridge;
        QSignalSpy stateSpy(&bridge, &FastRefreshBridge::stateChanged);

        bridge.markDevServerStarted(9099);
        QVERIFY(bridge.state() == BridgeState::Connecting || bridge.state() == BridgeState::Connected);
        QCOMPARE(bridge.port(), 9099);

        bridge.markDevServerStopped();
        QCOMPARE(bridge.state(), BridgeState::Idle);
    }

    void testStaleSessionOnPortMismatch() {
        FastRefreshBridge bridge;
        bridge.markDevServerStarted(8081);

        QSignalSpy staleSpy(&bridge, &FastRefreshBridge::staleSessionDetected);
        // Force state to connected
        bridge.processWebMessage(R"({"type":"page_loaded"})");
        QCOMPARE(bridge.state(), BridgeState::Connected);

        // Metro reports a different port while connected
        bridge.setMetroEndpoint("127.0.0.1", 8089);
        QCOMPARE(staleSpy.count(), 1);
        QVERIFY(staleSpy.first().at(0).toString().contains("port changed unexpectedly"));
    }
};
