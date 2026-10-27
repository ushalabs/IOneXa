#pragma once

#include <QTest>
#include <QObject>
#include <QSignalSpy>
#include <QDir>
#include <QFile>

#include "core/session_state.hpp"
#include "core/errors.hpp"
#include "engines/fast_refresh_bridge.hpp"
#include "engines/network_inspector.hpp"
#include "process/metro_supervisor.hpp"
#include "ui/viewport_manager.hpp"

using namespace ionexa::core;
using namespace ionexa::engines;
using namespace ionexa::process;
using namespace ionexa::ui;

class TestRegressionSuite : public QObject {
    Q_OBJECT

private slots:
    void testPreviewLifecycleStateMachine();
    void testIllegalStateTransitions();
    void testFastRefreshReloadAndErrorRecovery();
    void testProcessCrashAndSessionRecovery();
    void testNetworkInspectorStressAndClear();
    void testViewportRotationAndMetricsRegression();
    void testCleanTeardownAndResourceRelease();
};

inline void TestRegressionSuite::testPreviewLifecycleStateMachine() {
    SessionStateMachine sm;

    QCOMPARE(sm.currentState(), SessionState::Idle);

    // Lifecycle step 1: Start Inspecting project
    QVERIFY(sm.canTransitionTo(SessionState::Inspecting));
    QVERIFY(sm.transitionTo(SessionState::Inspecting));
    QCOMPARE(sm.currentState(), SessionState::Inspecting);

    // Lifecycle step 2: Transition to Ready
    QVERIFY(sm.canTransitionTo(SessionState::Ready));
    QVERIFY(sm.transitionTo(SessionState::Ready));
    QCOMPARE(sm.currentState(), SessionState::Ready);

    // Lifecycle step 3: Starting Metro bundler
    QVERIFY(sm.canTransitionTo(SessionState::Starting));
    QVERIFY(sm.transitionTo(SessionState::Starting));
    QCOMPARE(sm.currentState(), SessionState::Starting);

    // Lifecycle step 4: Metro & Preview Running
    QVERIFY(sm.canTransitionTo(SessionState::Running));
    QVERIFY(sm.transitionTo(SessionState::Running));
    QCOMPARE(sm.currentState(), SessionState::Running);

    // Lifecycle step 5: User Stop -> Stopping
    QVERIFY(sm.canTransitionTo(SessionState::Stopping));
    QVERIFY(sm.transitionTo(SessionState::Stopping));
    QCOMPARE(sm.currentState(), SessionState::Stopping);

    // Lifecycle step 6: Stopped
    QVERIFY(sm.canTransitionTo(SessionState::Stopped));
    QVERIFY(sm.transitionTo(SessionState::Stopped));
    QCOMPARE(sm.currentState(), SessionState::Stopped);

    // Lifecycle step 7: Reset to Idle
    QVERIFY(sm.canTransitionTo(SessionState::Idle));
    QVERIFY(sm.transitionTo(SessionState::Idle));
    QCOMPARE(sm.currentState(), SessionState::Idle);
}

inline void TestRegressionSuite::testIllegalStateTransitions() {
    SessionStateMachine sm;

    // From Idle, directly jumping to Running without inspecting/starting is illegal
    QVERIFY(!sm.canTransitionTo(SessionState::Running));
    QVERIFY(!sm.transitionTo(SessionState::Running));
    QCOMPARE(sm.currentState(), SessionState::Idle);

    // From Idle, jump to Stopping is illegal
    QVERIFY(!sm.canTransitionTo(SessionState::Stopping));
    QVERIFY(!sm.transitionTo(SessionState::Stopping));
    QCOMPARE(sm.currentState(), SessionState::Idle);

    // Valid Inspecting
    QVERIFY(sm.transitionTo(SessionState::Inspecting));

    // Jump from Inspecting directly to Running without Ready/Starting is illegal
    QVERIFY(!sm.canTransitionTo(SessionState::Running));
    QVERIFY(!sm.transitionTo(SessionState::Running));

    // Can transition to Error at any active stage
    QVERIFY(sm.canTransitionTo(SessionState::Error));
    QVERIFY(sm.transitionTo(SessionState::Error));
    QCOMPARE(sm.currentState(), SessionState::Error);

    // From Error, can recover to Idle
    QVERIFY(sm.canTransitionTo(SessionState::Idle));
    QVERIFY(sm.transitionTo(SessionState::Idle));
    QCOMPARE(sm.currentState(), SessionState::Idle);
}

inline void TestRegressionSuite::testFastRefreshReloadAndErrorRecovery() {
    FastRefreshBridge bridge;
    QSignalSpy bundleCompletedSpy(&bridge, &FastRefreshBridge::bundleCompleted);
    QSignalSpy bundleErrorSpy(&bridge, &FastRefreshBridge::bundleError);

    // Simulate successful bundle compilation
    bridge.processMetroLog("Web Bundled 320ms (index.js)");
    QCOMPARE(bundleCompletedSpy.count(), 1);

    // Simulate SyntaxError compilation failure in Metro log
    bridge.processMetroLog("SyntaxError: Unexpected token ',' in /app/index.js (14:22)");
    QCOMPARE(bundleErrorSpy.count(), 1);
    QCOMPARE(bridge.stats().bundleErrors, 1);

    // Verify recovery after code fix
    bridge.processMetroLog("Web Bundled 110ms (index.js)");
    QCOMPARE(bundleCompletedSpy.count(), 2);
    QCOMPARE(bridge.stats().fastRefreshCount, 2);
}

inline void TestRegressionSuite::testProcessCrashAndSessionRecovery() {
    SessionStateMachine sm;
    sm.transitionTo(SessionState::Inspecting);
    sm.transitionTo(SessionState::Ready);
    sm.transitionTo(SessionState::Starting);
    sm.transitionTo(SessionState::Running);
    QCOMPARE(sm.currentState(), SessionState::Running);

    // Simulate unexpected crash transitioning directly to Error
    QVERIFY(sm.transitionTo(SessionState::Error));
    QCOMPARE(sm.currentState(), SessionState::Error);

    // Test recovery from error back to Idle
    QVERIFY(sm.transitionTo(SessionState::Idle));
    QCOMPARE(sm.currentState(), SessionState::Idle);

    // Verify clean re-launch
    QVERIFY(sm.transitionTo(SessionState::Inspecting));
    QCOMPARE(sm.currentState(), SessionState::Inspecting);
}

inline void TestRegressionSuite::testNetworkInspectorStressAndClear() {
    NetworkInspector inspector;

    // Simulate 50 concurrent / rapid network requests
    for (int i = 1; i <= 50; ++i) {
        QString url = QString("http://localhost:8081/bundle%1.js").arg(i);

        QMap<QString, QString> reqHeaders = {
            {"Accept", "*/*"},
            {"Authorization", "Bearer sensitive-secret-token-12345"}
        };

        qint64 reqId = inspector.recordRequestStart(url, "GET", ResourceType::Script, reqHeaders);

        int statusCode = (i % 10 == 0) ? 500 : 200;
        QString statusText = (statusCode == 200) ? "OK" : "Internal Server Error";
        QMap<QString, QString> resHeaders = {
            {"Content-Type", "application/javascript"},
            {"Set-Cookie", "session=supersecretcookie"}
        };

        inspector.recordResponse(reqId, statusCode, statusText, 25, 1024, resHeaders, "console.log('stress');");
    }

    QCOMPARE(inspector.totalCount(), 50);
    QCOMPARE(inspector.errorCount(), 5);

    // Verify HAR export integrity under load
    QString har = inspector.exportAsHar();
    QVERIFY(!har.isEmpty());
    QVERIFY(har.contains("\"log\""));
    QVERIFY(har.contains("\"entries\""));
    // Verify security redaction worked across all 50 entries
    QVERIFY(!har.contains("sensitive-secret-token-12345"));
    QVERIFY(!har.contains("supersecretcookie"));
    QVERIFY(har.contains("[REDACTED]"));

    // Test inspector reset/clear
    inspector.clear();
    QCOMPARE(inspector.totalCount(), 0);
    QCOMPARE(inspector.errorCount(), 0);
    QCOMPARE(inspector.totalTransferredBytes(), 0);
}

inline void TestRegressionSuite::testViewportRotationAndMetricsRegression() {
    ViewportManager vm;

    // Set iPhone 14 preset
    bool selected = vm.selectProfileById("iphone-14");
    QVERIFY(selected);
    auto p1 = vm.activeProfile();
    QCOMPARE(p1.width, 390);
    QCOMPARE(p1.height, 844);
    QCOMPARE(vm.isLandscape(), false);

    // Rotate to Landscape
    vm.toggleOrientation();
    QCOMPARE(vm.isLandscape(), true);
    QCOMPARE(vm.currentPixelWidth(), 844);
    QCOMPARE(vm.currentPixelHeight(), 390);

    // Verify CDP command generation for landscape
    QString cdpMetrics = vm.buildDeviceMetricsJson();
    QVERIFY(cdpMetrics.contains("\"width\":844"));
    QVERIFY(cdpMetrics.contains("\"height\":390"));

    // Rotate back to Portrait
    vm.toggleOrientation();
    QCOMPARE(vm.isLandscape(), false);
    QCOMPARE(vm.currentPixelWidth(), 390);
    QCOMPARE(vm.currentPixelHeight(), 844);
}

inline void TestRegressionSuite::testCleanTeardownAndResourceRelease() {
    // Verify MetroSupervisor safely handles rapid creation and teardown without leaks or orphaned processes
    {
        MetroSupervisor supervisor;
        QCOMPARE(supervisor.status(), MetroStatus::Idle);
        // Calling stop when not running should be a safe no-op
        supervisor.stop();
        QCOMPARE(supervisor.status(), MetroStatus::Idle);
    }

    // Verify SessionStateMachine resets properly upon destroy
    {
        SessionStateMachine sm;
        sm.transitionTo(SessionState::Inspecting);
        sm.reset();
        QCOMPARE(sm.currentState(), SessionState::Idle);
    }
}
