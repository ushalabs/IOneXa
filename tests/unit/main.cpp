#include <QCoreApplication>
#include <QTest>
#include <iostream>

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    Q_UNUSED(type);
    Q_UNUSED(context);
    std::cout << "[QtLog] " << msg.toStdString() << std::endl;
}

#include "test_session_state.hpp"
#include "test_errors.hpp"
#include "test_capability_manifest.hpp"
#include "test_structured_logger.hpp"
#include "test_task_coordinator.hpp"
#include "test_app_settings.hpp"
#include "test_project_scanner.hpp"
#include "test_metro_supervisor.hpp"

int main(int argc, char* argv[]) {
    qInstallMessageHandler(myMessageOutput);
    QCoreApplication app(argc, argv);
    int status = 0;

    {
        TestSessionState tc;
        status |= QTest::qExec(&tc, argc, argv);
    }
    {
        TestErrors tc;
        status |= QTest::qExec(&tc, argc, argv);
    }
    {
        TestCapabilityManifest tc;
        status |= QTest::qExec(&tc, argc, argv);
    }
    {
        TestStructuredLogger tc;
        status |= QTest::qExec(&tc, argc, argv);
    }
    {
        TestTaskCoordinator tc;
        status |= QTest::qExec(&tc, argc, argv);
    }
    {
        TestAppSettings tc;
        status |= QTest::qExec(&tc, argc, argv);
    }
    {
        TestProjectScanner tc;
        status |= QTest::qExec(&tc, argc, argv);
    }
    {
        TestMetroSupervisor tc;
        int res = QTest::qExec(&tc, argc, argv);
        if (res != 0) std::cerr << "TestMetroSupervisor FAILED with code " << res << std::endl;
        status |= res;
    }

    if (status == 0) {
        std::cout << "\n============================================\n";
        std::cout << "ALL 8 IONEXA UNIT TEST SUITES PASSED (100%)!\n";
        std::cout << "============================================\n";
    }

    return status;
}
