#include <QCoreApplication>
#include <QTest>
#include <iostream>

#include "test_session_state.hpp"
#include "test_errors.hpp"
#include "test_capability_manifest.hpp"
#include "test_structured_logger.hpp"
#include "test_task_coordinator.hpp"
#include "test_app_settings.hpp"

int main(int argc, char* argv[]) {
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

    if (status == 0) {
        std::cout << "\n============================================\n";
        std::cout << "ALL 6 IONEXA UNIT TEST SUITES PASSED (100%)!\n";
        std::cout << "============================================\n";
    }

    return status;
}
