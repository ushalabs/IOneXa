#pragma once

#include <QTest>
#include <QObject>
#include <QSignalSpy>
#include <QClipboard>
#include <QApplication>

#include "ui/inspector_panel.hpp"
#include "ui/main_window.hpp"
#include "process/metro_supervisor.hpp"
#include "engines/fast_refresh_bridge.hpp"

using namespace ionexa::ui;
using namespace ionexa::process;
using namespace ionexa::engines;

class TestDebuggingWorkspace : public QObject {
    Q_OBJECT

private slots:
    void testInspectorStatusSegregation();
    void testInspectorSettingsPersistence();
    void testInspectorCopyEndpoint();
    void testWorkbenchInspectorIntegration();
};

inline void TestDebuggingWorkspace::testInspectorStatusSegregation() {
    InspectorPanel panel;

    // Test Metro state updates
    panel.updateProcess(1234, 8081, MetroStatus::Ready);
    auto* processStatus = panel.findChild<QLabel*>();
    QVERIFY(processStatus != nullptr);

    // Test Renderer state updates
    panel.updateRendererStatus("Active", true);

    // Test Fast Refresh HMR state updates
    FastRefreshStats stats;
    stats.fastRefreshCount = 5;
    stats.totalReloads = 1;
    stats.lastRefreshTime = QDateTime::currentDateTime();
    panel.updateFastRefresh(stats, BridgeState::Connected);

    QVERIFY(panel.width() >= 240);
}

inline void TestDebuggingWorkspace::testInspectorSettingsPersistence() {
    {
        InspectorPanel panel;
        panel.setFixedWidth(320);
        panel.saveSettings();
    }

    {
        InspectorPanel panel2;
        panel2.loadSettings();
        QCOMPARE(panel2.width(), 320);

        // Reset to default
        panel2.setFixedWidth(290);
        panel2.saveSettings();
    }
}

inline void TestDebuggingWorkspace::testInspectorCopyEndpoint() {
    InspectorPanel panel;
    panel.updateProcess(4321, 8088, MetroStatus::Ready);

    auto* btnCopy = panel.findChild<QPushButton*>();
    QVERIFY(btnCopy != nullptr);
}

inline void TestDebuggingWorkspace::testWorkbenchInspectorIntegration() {
    MainWindow window;
    window.show();

    // Verify Inspector starts visible
    QVERIFY(!window.inspectorPanel()->isHidden());

    // Toggle to hide
    window.toggleInspector();
    QVERIFY(window.inspectorPanel()->isHidden());

    // Toggle to show
    window.toggleInspector();
    QVERIFY(!window.inspectorPanel()->isHidden());
}
