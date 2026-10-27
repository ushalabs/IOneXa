#include <QCoreApplication>
#include <QTest>
#include <iostream>

#include "test_session_state.hpp"
#include "test_errors.hpp"
#include "test_capability_manifest.hpp"

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

    if (status == 0) {
        std::cout << "\n============================================\n";
        std::cout << "ALL IONEXA UNIT TESTS PASSED SUCCESSFULLY!\n";
        std::cout << "============================================\n";
    }

    return status;
}
