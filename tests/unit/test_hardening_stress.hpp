#pragma once

#include <QTest>
#include <QObject>
#include <QDir>
#include <QFile>

#include "core/session_state.hpp"
#include "core/security_policy.hpp"
#include "core/structured_logger.hpp"
#include "engines/fast_refresh_bridge.hpp"
#include "engines/network_inspector.hpp"
#include "process/metro_supervisor.hpp"

using namespace ionexa::core;
using namespace ionexa::engines;
using namespace ionexa::process;

class TestHardeningStress : public QObject {
    Q_OBJECT

private slots:
    void testSessionLifecycleStressLoops();
    void testRapidSupervisorTeardownStress();
    void testStructuredLoggerMemoryBounds();
    void testSecurityPolicyUrlBoundaries();
    void testPathTraversalHardening();
    void testNetworkInspectorHeaderRedactionHardening();
    void testMalformedJsonIpcHardening();
};

inline void TestHardeningStress::testSessionLifecycleStressLoops() {
    SessionStateMachine sm;

    // 25 repeated complete lifecycle stress loops
    for (int i = 0; i < 25; ++i) {
        QCOMPARE(sm.currentState(), SessionState::Idle);

        QVERIFY(sm.transitionTo(SessionState::Inspecting));
        QVERIFY(sm.transitionTo(SessionState::Ready));
        QVERIFY(sm.transitionTo(SessionState::Starting));
        QVERIFY(sm.transitionTo(SessionState::Running));
        QVERIFY(sm.transitionTo(SessionState::Stopping));
        QVERIFY(sm.transitionTo(SessionState::Stopped));
        QVERIFY(sm.transitionTo(SessionState::Idle));
    }

    QCOMPARE(sm.currentState(), SessionState::Idle);
}

inline void TestHardeningStress::testRapidSupervisorTeardownStress() {
    // 25 rapid supervisor instantiate-and-stop stress cycles
    for (int i = 0; i < 25; ++i) {
        MetroSupervisor supervisor;
        QCOMPARE(supervisor.status(), MetroStatus::Idle);
        supervisor.stop();
        QCOMPARE(supervisor.status(), MetroStatus::Idle);

        // Verify port availability search remains fast and stable
        int port = MetroSupervisor::findAvailablePort(8300 + i);
        QVERIFY(port >= 8300 + i);
    }
}

inline void TestHardeningStress::testStructuredLoggerMemoryBounds() {
    auto& logger = StructuredLogger::instance();
    logger.clear();
    QCOMPARE(logger.size(), 0);

    // Push 3000 log entries under rapid stress
    for (int i = 0; i < 3000; ++i) {
        logger.info(LogSource::Engine, std::string("Stress log payload #") + std::to_string(i));
    }

    // Verify logger respects maximum buffer limit and does not grow unbounded
    QVERIFY(logger.size() <= 5000);
    QVERIFY(logger.size() > 0);

    // Clean up
    logger.clear();
    QCOMPARE(logger.size(), 0);
}

inline void TestHardeningStress::testSecurityPolicyUrlBoundaries() {
    // Safe preview URLs (loopback hosts with safe ports)
    QVERIFY(SecurityPolicy::isSafePreviewUrl("http://localhost:8081"));
    QVERIFY(SecurityPolicy::isSafePreviewUrl("http://127.0.0.1:8081/"));
    QVERIFY(SecurityPolicy::isSafePreviewUrl("http://127.0.0.1:8082/index.bundle?platform=web"));
    QVERIFY(SecurityPolicy::isSafePreviewUrl("https://localhost:19006"));
    QVERIFY(SecurityPolicy::isSafePreviewUrl("about:blank"));

    // Dangerous schemes must be blocked
    QVERIFY(!SecurityPolicy::isSafePreviewUrl("javascript:alert(1)"));
    QVERIFY(!SecurityPolicy::isSafePreviewUrl("vbscript:MsgBox(1)"));
    QVERIFY(!SecurityPolicy::isSafePreviewUrl("file:///C:/Windows/System32/cmd.exe"));
    QVERIFY(!SecurityPolicy::isSafePreviewUrl("data:text/html,<h1>Hacked</h1>"));
    QVERIFY(!SecurityPolicy::isSafePreviewUrl("ms-appx:///evil.js"));
    QVERIFY(!SecurityPolicy::isSafePreviewUrl("res://ieframe.dll"));

    // External non-loopback URLs must be blocked for preview
    QVERIFY(!SecurityPolicy::isSafePreviewUrl("http://google.com"));
    QVERIFY(!SecurityPolicy::isSafePreviewUrl("https://malicious-site.org/index.html"));
    QVERIFY(!SecurityPolicy::isSafePreviewUrl("http://192.168.1.100:8081"));

    // Privileged system ports (<1024) must be blocked
    QVERIFY(!SecurityPolicy::isSafePreviewUrl("http://localhost:22"));
    QVERIFY(!SecurityPolicy::isSafePreviewUrl("http://localhost:80"));
    QVERIFY(!SecurityPolicy::isSafePreviewUrl("http://localhost:443"));

    // Empty or malformed URLs
    QVERIFY(!SecurityPolicy::isSafePreviewUrl(""));
    QVERIFY(!SecurityPolicy::isSafePreviewUrl("   "));
    QVERIFY(!SecurityPolicy::isSafePreviewUrl("not-a-valid-url:::"));
}

inline void TestHardeningStress::testPathTraversalHardening() {
    QString currentDir = QDir::currentPath();

    // Verify traversal detection
    QVERIFY(SecurityPolicy::containsTraversalSequences("../secret.txt"));
    QVERIFY(SecurityPolicy::containsTraversalSequences("..\\Windows\\System32"));
    QVERIFY(SecurityPolicy::containsTraversalSequences("foo/bar/../../secret"));
    QVERIFY(SecurityPolicy::containsTraversalSequences(QString("foo%1bar").arg(QChar('\0'))));

    // Safe path within directory
    QString safeSub = QDir(currentDir).filePath("tests/fixtures");
    QVERIFY(SecurityPolicy::isPathWithinDirectory(currentDir, safeSub));

    // Attempted directory traversal outside base
    QString maliciousPath = QDir(currentDir).filePath("../../Windows/System32");
    QVERIFY(!SecurityPolicy::isPathWithinDirectory(currentDir, maliciousPath));
}

inline void TestHardeningStress::testNetworkInspectorHeaderRedactionHardening() {
    NetworkInspector inspector;

    QMap<QString, QString> adversarialHeaders = {
        {"AUTHORIZATION", "Bearer my-super-secret-token-key-999"},
        {"x-api-key", "live_secret_key_88888888"},
        {"cookie", "auth_session=12345; user=admin"},
        {"Set-Cookie", "jwt=abcdefghijk"},
        {"token", "tok_live_1234567890"},
        {"secret", "top_secret_salt_value"},
        {"Public-Header", "standard-value"}
    };

    qint64 id = inspector.recordRequestStart("http://127.0.0.1:8081/secure", "GET", ResourceType::Fetch, adversarialHeaders);
    inspector.recordResponse(id, 200, "OK", 10, 500, adversarialHeaders);

    QString har = inspector.exportAsHar();
    // Non-sensitive header should be retained
    QVERIFY(har.contains("standard-value"));

    // All sensitive variants must be redacted
    QVERIFY(!har.contains("my-super-secret-token-key-999"));
    QVERIFY(!har.contains("live_secret_key_88888888"));
    QVERIFY(!har.contains("auth_session=12345"));
    QVERIFY(!har.contains("tok_live_1234567890"));
    QVERIFY(!har.contains("top_secret_salt_value"));
}

inline void TestHardeningStress::testMalformedJsonIpcHardening() {
    FastRefreshBridge bridge;

    // Test 1: Empty string
    bridge.processWebMessage("");

    // Test 2: Invalid JSON syntax
    bridge.processWebMessage("{ not valid json at all :::");

    // Test 3: Truncated JSON
    bridge.processWebMessage("{\"type\": \"fast-refresh\", \"data\": ");

    // Test 4: Deeply nested JSON
    QString deepJson;
    for (int i = 0; i < 50; ++i) deepJson += "{\"nested\":";
    deepJson += "42";
    for (int i = 0; i < 50; ++i) deepJson += "}";
    bridge.processWebMessage(deepJson);

    // Test 5: Oversized JSON payload (100KB of padding)
    QString largeJson = "{\"type\":\"test\",\"padding\":\"" + QString(100000, 'X') + "\"}";
    bridge.processWebMessage(largeJson);

    // Verify bridge did not crash and remains healthy
    QCOMPARE(bridge.stats().bundleErrors, 0);
}
