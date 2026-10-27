#pragma once

#include <QTest>
#include "core/structured_logger.hpp"

using namespace ionexa::core;

class TestStructuredLogger : public QObject {
    Q_OBJECT

private slots:
    void testLoggingAndFiltering() {
        StructuredLogger logger(100);
        logger.clear();

        logger.info(LogSource::Metro, "Bundler ready on port 8081");
        logger.debug(LogSource::BrowserConsole, "React DOM mounted");
        logger.error(LogSource::Engine, "Connection reset");

        QCOMPARE(logger.size(), static_cast<size_t>(3));

        auto allEntries = logger.getEntries(LogLevel::Debug);
        QCOMPARE(allEntries.size(), static_cast<size_t>(3));

        auto errorOnly = logger.getEntries(LogLevel::Error);
        QCOMPARE(errorOnly.size(), static_cast<size_t>(1));
        QCOMPARE(QString::fromStdString(errorOnly[0].message), QString("Connection reset"));

        LogSource metroSrc = LogSource::Metro;
        auto metroOnly = logger.getEntries(LogLevel::Debug, &metroSrc);
        QCOMPARE(metroOnly.size(), static_cast<size_t>(1));
        QCOMPARE(QString::fromStdString(metroOnly[0].message), QString("Bundler ready on port 8081"));
    }

    void testSubscriberCallback() {
        StructuredLogger logger(50);
        logger.clear();

        int notifiedCount = 0;
        std::string lastMsg;

        logger.addSubscriber([&](const LogEntry& entry) {
            ++notifiedCount;
            lastMsg = entry.message;
        });

        logger.warn(LogSource::System, "High memory usage");
        QCOMPARE(notifiedCount, 1);
        QCOMPARE(QString::fromStdString(lastMsg), QString("High memory usage"));
    }
};
