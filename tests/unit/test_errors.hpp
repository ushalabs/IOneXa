#pragma once

#include <QTest>
#include "core/errors.hpp"

using namespace ionexa::core;

class TestErrors : public QObject {
    Q_OBJECT

private slots:
    void testErrorCreation() {
        IOneXaError err(ErrorCategory::General, 100, "General failure", "Context line");
        QVERIFY(err.category() == ErrorCategory::General);
        QCOMPARE(err.code(), 100);
        QCOMPARE(QString::fromUtf8(err.what()), QString("General failure"));
        QCOMPARE(QString::fromStdString(err.details()), QString("Context line"));
    }

    void testDerivedErrors() {
        ProjectError pe(101, "Missing package.json");
        QVERIFY(pe.category() == ErrorCategory::ProjectInspection);
        QCOMPARE(pe.code(), 101);

        ProcessError proce(201, "Port collision");
        QVERIFY(proce.category() == ErrorCategory::ProcessManagement);

        SessionError se(301, "Invalid state jump");
        QVERIFY(se.category() == ErrorCategory::SessionLifecycle);
    }

    void testFormatting() {
        ProjectError pe(102, "Expo SDK version missing", "root=C:/App");
        std::string formatted = pe.format();
        QVERIFY(formatted.find("[ProjectInspection E102]") != std::string::npos);
        QVERIFY(formatted.find("Expo SDK version missing") != std::string::npos);
        QVERIFY(formatted.find("root=C:/App") != std::string::npos);
    }
};
