#pragma once

#include <QTest>
#include <QSignalSpy>
#include <QTcpServer>
#include "process/metro_supervisor.hpp"
#include "project/project_scanner.hpp"

#ifndef PROJECT_ROOT_DIR
#define PROJECT_ROOT_DIR "."
#endif

using namespace ionexa::process;
using namespace ionexa::project;

class TestMetroSupervisor : public QObject {
    Q_OBJECT

private:
    QString getRootDir() const {
        return QString::fromUtf8(PROJECT_ROOT_DIR);
    }

private slots:
    void testPortConflictResolution() {
        qDebug() << "Testing port conflict resolution...";
        int p1 = MetroSupervisor::findAvailablePort(8200);
        QVERIFY(p1 >= 8200);

        QTcpServer blocker;
        QVERIFY(blocker.listen(QHostAddress::LocalHost, static_cast<quint16>(p1)));

        int p2 = MetroSupervisor::findAvailablePort(p1);
        qDebug() << "p1 was:" << p1 << "p2 found:" << p2;
        QVERIFY(p2 > p1);

        blocker.close();
        qDebug() << "Port conflict resolution PASSED.";
    }

    void testStartAndStopLifecycle() {
        qDebug() << "Testing start and stop lifecycle...";
        QString fixturePath = getRootDir() + "/tests/fixtures/expo-basic";
        ProjectDetails details = ProjectScanner::scan(fixturePath.toStdString());
        QVERIFY(details.isValid);
        QVERIFY(details.isExpoProject);

        MetroSupervisor supervisor;
        QSignalSpy statusSpy(&supervisor, &MetroSupervisor::statusChanged);
        QSignalSpy logSpy(&supervisor, &MetroSupervisor::logOutput);

        int targetPort = MetroSupervisor::findAvailablePort(8350);
        qDebug() << "Target port chosen:" << targetPort;
        bool started = supervisor.start(details, targetPort);
        qDebug() << "supervisor.start returned:" << started;
        QVERIFY(started);

        qDebug() << "supervisor status:" << (int)supervisor.status() << "pid:" << supervisor.processId();
        QCOMPARE(supervisor.status(), MetroStatus::Starting);
        QVERIFY(supervisor.processId() > 0);
        QCOMPARE(supervisor.activePort(), targetPort);

        // Wait up to 15 seconds for supervisor to spawn and stream logs
        for (int i = 0; i < 30; ++i) {
            QTest::qWait(500);
            if (logSpy.count() > 0) break;
        }

        qDebug() << "Log count received:" << logSpy.count();
        QVERIFY(logSpy.count() > 0);

        // Stop supervisor
        QVERIFY(supervisor.stop(3000));
        QCOMPARE(supervisor.status(), MetroStatus::Stopped);
    }
};
