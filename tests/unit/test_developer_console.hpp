#pragma once

#include <QTest>
#include <QSignalSpy>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include "ui/developer_console_panel.hpp"

using namespace ionexa::ui;

class TestDeveloperConsole : public QObject {
    Q_OBJECT

private slots:
    void testLogEntryStorage() {
        DeveloperConsolePanel panel;
        QCOMPARE(panel.totalCount(), 0);

        QSignalSpy addSpy(&panel, &DeveloperConsolePanel::logAdded);

        panel.appendLog(ConsoleLogLevel::Info, ConsoleLogSource::Metro, "Metro dev server ready");
        panel.appendLog(ConsoleLogLevel::Warning, ConsoleLogSource::Browser, "Deprecated component used");
        panel.appendLog(ConsoleLogLevel::Error, ConsoleLogSource::System, "Socket bind timeout");

        QCOMPARE(panel.totalCount(), 3);
        QCOMPARE(addSpy.count(), 3);

        const auto& entries = panel.allEntries();
        QCOMPARE(entries[0].message, QString("Metro dev server ready"));
        QCOMPARE(entries[0].source, ConsoleLogSource::Metro);
        QCOMPARE(entries[0].level, ConsoleLogLevel::Info);

        QCOMPARE(entries[1].message, QString("Deprecated component used"));
        QCOMPARE(entries[1].source, ConsoleLogSource::Browser);
        QCOMPARE(entries[1].level, ConsoleLogLevel::Warning);

        QCOMPARE(entries[2].message, QString("Socket bind timeout"));
        QCOMPARE(entries[2].source, ConsoleLogSource::System);
        QCOMPARE(entries[2].level, ConsoleLogLevel::Error);
    }

    void testSeverityCounters() {
        DeveloperConsolePanel panel;

        panel.appendLog(ConsoleLogLevel::Error, ConsoleLogSource::Metro, "Error 1");
        panel.appendLog(ConsoleLogLevel::Error, ConsoleLogSource::Browser, "Error 2");
        panel.appendLog(ConsoleLogLevel::Warning, ConsoleLogSource::Metro, "Warning 1");
        panel.appendLog(ConsoleLogLevel::Warning, ConsoleLogSource::Browser, "Warning 2");
        panel.appendLog(ConsoleLogLevel::Warning, ConsoleLogSource::Browser, "Warning 3");
        panel.appendLog(ConsoleLogLevel::Info, ConsoleLogSource::System, "Info 1");

        QCOMPARE(panel.countByLevel(ConsoleLogLevel::Error), 2);
        QCOMPARE(panel.countByLevel(ConsoleLogLevel::Warning), 3);
        QCOMPARE(panel.countByLevel(ConsoleLogLevel::Info), 1);
        QCOMPARE(panel.countByLevel(ConsoleLogLevel::Debug), 0);
    }

    void testFilteringBySeverity() {
        DeveloperConsolePanel panel;
        panel.appendLog(ConsoleLogLevel::Info, ConsoleLogSource::Metro, "Info message");
        panel.appendLog(ConsoleLogLevel::Warning, ConsoleLogSource::Metro, "Warning message");
        panel.appendLog(ConsoleLogLevel::Error, ConsoleLogSource::Metro, "Error message");

        // Filter errors only
        panel.setFilterLevel(ConsoleLogLevel::Error);
        auto errors = panel.filteredEntries();
        QCOMPARE(errors.size(), 1);
        QCOMPARE(errors[0].message, QString("Error message"));

        // Filter warnings and errors
        panel.setFilterLevel(ConsoleLogLevel::Warning);
        auto warnsAndErrors = panel.filteredEntries();
        QCOMPARE(warnsAndErrors.size(), 2);

        // Reset to all
        panel.setFilterLevel(ConsoleLogLevel::All);
        QCOMPARE(panel.filteredEntries().size(), 3);
    }

    void testFilteringBySourceAndSearch() {
        DeveloperConsolePanel panel;
        panel.appendLog(ConsoleLogLevel::Info, ConsoleLogSource::Metro, "Bundling index.js");
        panel.appendLog(ConsoleLogLevel::Info, ConsoleLogSource::Browser, "Fast Refresh applied");
        panel.appendLog(ConsoleLogLevel::Error, ConsoleLogSource::Browser, "Syntax error in App.js");

        // Filter by browser source
        panel.setFilterSource(ConsoleLogSource::Browser);
        QCOMPARE(panel.filteredEntries().size(), 2);

        // Filter by substring search
        panel.setFilterText("syntax");
        auto filtered = panel.filteredEntries();
        QCOMPARE(filtered.size(), 1);
        QCOMPARE(filtered[0].message, QString("Syntax error in App.js"));

        // Reset
        panel.setFilterSource(ConsoleLogSource::All);
        panel.setFilterText("");
        QCOMPARE(panel.filteredEntries().size(), 3);
    }

    void testTextAndJsonExport() {
        DeveloperConsolePanel panel;
        panel.appendLog(ConsoleLogLevel::Info, ConsoleLogSource::Metro, "Server running on 8081");
        panel.appendLog(ConsoleLogLevel::Error, ConsoleLogSource::Browser, "Uncaught exception", "at App.js:42");

        // Text export
        QString text = panel.exportAsText();
        QVERIFY(text.contains("Server running on 8081"));
        QVERIFY(text.contains("Uncaught exception"));
        QVERIFY(text.contains("[Metro]"));
        QVERIFY(text.contains("[Browser]"));

        // JSON export
        QString jsonStr = panel.exportAsJson();
        QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
        QVERIFY(doc.isArray());
        QJsonArray arr = doc.array();
        QCOMPARE(arr.size(), 2);

        QJsonObject obj1 = arr[0].toObject();
        QCOMPARE(obj1.value("message").toString(), QString("Server running on 8081"));
        QCOMPARE(obj1.value("source").toInt(), static_cast<int>(ConsoleLogSource::Metro));

        QJsonObject obj2 = arr[1].toObject();
        QCOMPARE(obj2.value("message").toString(), QString("Uncaught exception"));
        QCOMPARE(obj2.value("stackTrace").toString(), QString("at App.js:42"));
    }

    void testClearConsole() {
        DeveloperConsolePanel panel;
        panel.appendLog(ConsoleLogLevel::Info, ConsoleLogSource::System, "Hello");
        QCOMPARE(panel.totalCount(), 1);

        QSignalSpy clearSpy(&panel, &DeveloperConsolePanel::consoleCleared);
        panel.clear();

        QCOMPARE(panel.totalCount(), 0);
        QCOMPARE(panel.countByLevel(ConsoleLogLevel::Info), 0);
        QCOMPARE(clearSpy.count(), 1);
    }
};
