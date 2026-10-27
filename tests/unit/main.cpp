#include <QApplication>
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
#include "test_webview2_engine.hpp"
#include "test_ui_components.hpp"
#include "test_fast_refresh.hpp"
#include "test_viewport_presets.hpp"
#include "test_developer_console.hpp"
#include "test_network_inspector.hpp"
#include "test_compatibility_analyzer.hpp"
#include "test_feature_suite.hpp"
#include "test_regression_suite.hpp"
#include "test_hardening_stress.hpp"
#include "test_handoff_generator.hpp"
#include "test_gui_architecture.hpp"
#include "test_emulator_window.hpp"
#include "test_workbench_views.hpp"

int main(int argc, char* argv[]) {
    Q_UNUSED(argc);
    Q_UNUSED(argv);
    // qInstallMessageHandler(myMessageOutput);
    QApplication app(argc, argv);
    int status = 0;

    auto runSuite = [&](const char* name, auto& tc) {
        char* args[] = {(char*)"test", (char*)"-v1"};
        int r = QTest::qExec(&tc, 2, args);
        if (r != 0) {
            std::cerr << "FAILED: " << name << " (code " << r << ")" << std::endl;
        } else {
            std::cout << "PASSED: " << name << std::endl;
        }
        status |= r;
    };

    {
        TestSessionState tc;
        runSuite("TestSessionState", tc);
    }
    {
        TestErrors tc;
        runSuite("TestErrors", tc);
    }
    {
        TestCapabilityManifest tc;
        runSuite("TestCapabilityManifest", tc);
    }
    {
        TestStructuredLogger tc;
        runSuite("TestStructuredLogger", tc);
    }
    {
        TestTaskCoordinator tc;
        runSuite("TestTaskCoordinator", tc);
    }
    {
        TestAppSettings tc;
        runSuite("TestAppSettings", tc);
    }
    {
        TestProjectScanner tc;
        runSuite("TestProjectScanner", tc);
    }
    {
        TestMetroSupervisor tc;
        runSuite("TestMetroSupervisor", tc);
    }
    {
        TestWebView2Engine tc;
        runSuite("TestWebView2Engine", tc);
    }
    {
        TestUiComponents tc;
        runSuite("TestUiComponents", tc);
    }
    {
        TestFastRefresh tc;
        runSuite("TestFastRefresh", tc);
    }
    {
        TestViewportPresets tc;
        runSuite("TestViewportPresets", tc);
    }
    {
        TestDeveloperConsole tc;
        runSuite("TestDeveloperConsole", tc);
    }
    {
        TestNetworkInspector tc;
        runSuite("TestNetworkInspector", tc);
    }
    {
        TestCompatibilityAnalyzer tc;
        runSuite("TestCompatibilityAnalyzer", tc);
    }
    {
        TestFeatureSuite tc;
        runSuite("TestFeatureSuite", tc);
    }
    {
        TestRegressionSuite tc;
        runSuite("TestRegressionSuite", tc);
    }
    {
        TestHardeningStress tc;
        runSuite("TestHardeningStress", tc);
    }
    {
        TestHandoffGenerator tc;
        runSuite("TestHandoffGenerator", tc);
    }
    {
        TestGuiArchitecture tc;
        runSuite("TestGuiArchitecture", tc);
    }
    {
        TestEmulatorWindow tc;
        runSuite("TestEmulatorWindow", tc);
    }
    {
        TestWorkbenchViews tc;
        runSuite("TestWorkbenchViews", tc);
    }

    if (status == 0) {
        std::cout << "\n============================================\n";
        std::cout << "ALL 22 IONEXA UNIT TEST SUITES PASSED (100%)!\n";
        std::cout << "============================================\n";
    }

    return status;
}
