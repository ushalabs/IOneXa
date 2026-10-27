#pragma once

#include <QTest>
#include <QObject>
#include <QSignalSpy>

#include "ui/main_window.hpp"
#include "ui/nav_rail.hpp"

using namespace ionexa::ui;

class TestWorkbenchViews : public QObject {
    Q_OBJECT

private slots:
    void testWorkbenchInitialization();
    void testNavRailStackSynchronization();
    void testInspectorToggle();
    void testTruthDisclosureInvariant();
};

inline void TestWorkbenchViews::testWorkbenchInitialization() {
    MainWindow window;
    QVERIFY(window.windowTitle().contains("IOneXa"));
    QVERIFY(window.centralStack() != nullptr);
    QCOMPARE(window.centralStack()->count(), 7);
    QVERIFY(window.navRail() != nullptr);
    QVERIFY(window.emulatorWindow() != nullptr);
    QVERIFY(window.inspectorPanel() != nullptr);
}

inline void TestWorkbenchViews::testNavRailStackSynchronization() {
    MainWindow window;

    // Default section is Workspace/Home (0)
    QCOMPARE(window.navRail()->activeSection(), NavSection::Home);
    QCOMPARE(window.centralStack()->currentIndex(), 0);

    // Switch to Projects (1)
    window.navRail()->setActiveSection(NavSection::Projects);
    QCOMPARE(window.centralStack()->currentIndex(), 1);

    // Switch to Devices (2)
    window.navRail()->setActiveSection(NavSection::Devices);
    QCOMPARE(window.centralStack()->currentIndex(), 2);

    // Switch to Console (3)
    window.navRail()->setActiveSection(NavSection::Console);
    QCOMPARE(window.centralStack()->currentIndex(), 3);

    // Switch to Network (4)
    window.navRail()->setActiveSection(NavSection::Network);
    QCOMPARE(window.centralStack()->currentIndex(), 4);

    // Switch to Compatibility (5)
    window.navRail()->setActiveSection(NavSection::Compatibility);
    QCOMPARE(window.centralStack()->currentIndex(), 5);

    // Switch to Settings (6)
    window.navRail()->setActiveSection(NavSection::Settings);
    QCOMPARE(window.centralStack()->currentIndex(), 6);
}

inline void TestWorkbenchViews::testInspectorToggle() {
    MainWindow window;
    window.show();
    QVERIFY(!window.inspectorPanel()->isHidden());

    window.toggleInspector();
    QCOMPARE(window.inspectorPanel()->isHidden(), true);

    window.toggleInspector();
    QCOMPARE(window.inspectorPanel()->isHidden(), false);
}

inline void TestWorkbenchViews::testTruthDisclosureInvariant() {
    MainWindow window;
    // Window A (Workbench) must feature the truth-in-labeling badge
    auto* badge = window.findChild<QLabel*>("truthDisclosureBadge");
    QVERIFY(badge != nullptr);
    QVERIFY(badge->text().contains("Expo Web Preview — Not an iOS Runtime"));
}
