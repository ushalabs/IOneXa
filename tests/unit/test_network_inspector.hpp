#pragma once

#include <QTest>
#include <QSignalSpy>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "engines/network_inspector.hpp"

using namespace ionexa::engines;

class TestNetworkInspector : public QObject {
    Q_OBJECT

private slots:
    void testRecordRequestLifecycle() {
        NetworkInspector inspector;
        QCOMPARE(inspector.totalCount(), 0);

        QSignalSpy startSpy(&inspector, &NetworkInspector::requestStarted);
        QSignalSpy completeSpy(&inspector, &NetworkInspector::requestCompleted);

        QMap<QString, QString> reqHeaders;
        reqHeaders["Content-Type"] = "application/json";

        qint64 reqId = inspector.recordRequestStart("http://127.0.0.1:8081/index.bundle", "GET", ResourceType::Script, reqHeaders);
        QVERIFY(reqId > 0);
        QCOMPARE(inspector.totalCount(), 1);
        QCOMPARE(startSpy.count(), 1);

        QMap<QString, QString> resHeaders;
        resHeaders["Content-Type"] = "application/javascript";
        inspector.recordResponse(reqId, 200, "OK", 85, 45200, resHeaders, "console.log('loaded');");

        QCOMPARE(completeSpy.count(), 1);

        const auto& reqs = inspector.allRequests();
        QCOMPARE(reqs[0].statusCode, 200);
        QCOMPARE(reqs[0].durationMs, 85);
        QCOMPARE(reqs[0].responseBytes, 45200);
        QCOMPARE(reqs[0].isCompleted, true);
        QCOMPARE(reqs[0].isFailed, false);
    }

    void testSensitiveHeaderRedaction() {
        QMap<QString, QString> headers;
        headers["Authorization"] = "Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9";
        headers["Cookie"] = "session_token=abcdef123456; theme=dark";
        headers["X-API-Key"] = "sk-live-99887766";
        headers["Accept"] = "application/json";
        headers["User-Agent"] = "IOneXa/0.1.0";

        auto redacted = NetworkInspector::redactHeaders(headers);

        QCOMPARE(redacted.value("Authorization"), QString("Bearer [REDACTED]"));
        QCOMPARE(redacted.value("Cookie"), QString("[REDACTED]"));
        QCOMPARE(redacted.value("X-API-Key"), QString("[REDACTED]"));
        QCOMPARE(redacted.value("Accept"), QString("application/json"));
        QCOMPARE(redacted.value("User-Agent"), QString("IOneXa/0.1.0"));
    }

    void testDeduceResourceType() {
        QCOMPARE(NetworkInspector::deduceResourceType("http://localhost:8081/index.bundle?platform=web"), ResourceType::Script);
        QCOMPARE(NetworkInspector::deduceResourceType("http://localhost:8081/styles.css"), ResourceType::Stylesheet);
        QCOMPARE(NetworkInspector::deduceResourceType("http://localhost:8081/assets/logo.png"), ResourceType::Image);
        QCOMPARE(NetworkInspector::deduceResourceType("http://localhost:8081/assets/Inter.woff2"), ResourceType::Font);
        QCOMPARE(NetworkInspector::deduceResourceType("https://api.example.com/api/users"), ResourceType::Fetch);
    }

    void testFiltering() {
        NetworkInspector inspector;
        qint64 id1 = inspector.recordRequestStart("http://localhost:8081/app.js", "GET", ResourceType::Script);
        inspector.recordResponse(id1, 200, "OK", 50);

        qint64 id2 = inspector.recordRequestStart("http://localhost:8081/api/profile", "GET", ResourceType::Fetch);
        inspector.recordResponse(id2, 404, "Not Found", 30);

        qint64 id3 = inspector.recordRequestStart("http://localhost:8081/api/login", "POST", ResourceType::Fetch);
        inspector.recordResponse(id3, 200, "OK", 120);

        // Filter by resource type
        auto scripts = inspector.filteredRequests(ResourceType::Script);
        QCOMPARE(scripts.size(), 1);

        auto fetches = inspector.filteredRequests(ResourceType::Fetch);
        QCOMPARE(fetches.size(), 2);

        // Filter by status code (400 client error)
        auto errors = inspector.filteredRequests(ResourceType::All, 400);
        QCOMPARE(errors.size(), 1);
        QCOMPARE(errors[0].statusCode, 404);

        // Filter by URL substring
        auto profileSearch = inspector.filteredRequests(ResourceType::All, 0, "profile");
        QCOMPARE(profileSearch.size(), 1);
    }

    void testHarAndJsonExport() {
        NetworkInspector inspector;
        qint64 id = inspector.recordRequestStart("http://127.0.0.1:8081/api/items", "GET", ResourceType::Fetch);
        inspector.recordResponse(id, 200, "OK", 45, 1200, {{"content-type", "application/json"}}, "[{\"id\":1}]");

        // HAR export verification
        QString harStr = inspector.exportAsHar();
        QJsonDocument harDoc = QJsonDocument::fromJson(harStr.toUtf8());
        QVERIFY(harDoc.isObject());
        QJsonObject harLog = harDoc.object().value("log").toObject();
        QCOMPARE(harLog.value("version").toString(), QString("1.2"));
        QJsonArray harEntries = harLog.value("entries").toArray();
        QCOMPARE(harEntries.size(), 1);
        QCOMPARE(harEntries[0].toObject().value("request").toObject().value("method").toString(), QString("GET"));
        QCOMPARE(harEntries[0].toObject().value("response").toObject().value("status").toInt(), 200);

        // JSON export verification
        QString jsonStr = inspector.exportAsJson();
        QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
        QVERIFY(doc.isArray());
        QCOMPARE(doc.array().size(), 1);
    }

    void testClear() {
        NetworkInspector inspector;
        qint64 id = inspector.recordRequestStart("http://localhost:8081/", "GET", ResourceType::Document);
        inspector.recordResponse(id, 200, "OK", 10);
        QCOMPARE(inspector.totalCount(), 1);

        QSignalSpy clearSpy(&inspector, &NetworkInspector::cleared);
        inspector.clear();

        QCOMPARE(inspector.totalCount(), 0);
        QCOMPARE(clearSpy.count(), 1);
    }
};
