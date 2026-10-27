#pragma once

#include <QTest>
#include <QSignalSpy>
#include "ui/nav_rail.hpp"
#include "ui/inspector_panel.hpp"
#include "project/project_scanner.hpp"
#include "process/metro_supervisor.hpp"

using namespace ionexa::ui;
using namespace ionexa::project;
using namespace ionexa::process;

class TestUiComponents : public QObject {
    Q_OBJECT

private slots:
    void testNavRailSections() {
        NavRail rail;
        QCOMPARE(rail.activeSection(), NavSection::Workspace);

        QSignalSpy spy(&rail, &NavRail::sectionSelected);
        rail.setActiveSection(NavSection::Diagnostics);
        QCOMPARE(rail.activeSection(), NavSection::Diagnostics);
        QCOMPARE(spy.count(), 1);
    }

    void testInspectorPanelUpdates() {
        InspectorPanel panel;

        ionexa::project::ProjectDetails proj;
        proj.isValid = true;
        proj.isExpoProject = true;
        proj.projectName = "test-app";
        proj.expoSdkVersion = "~52.0.0";
        proj.packageManager = "yarn";
        proj.hasWebSupport = true;

        panel.updateProject(proj);
        panel.updateProcess(4521, 8085, ionexa::process::MetroStatus::Ready);
        panel.updateViewport(430, 932, false, 1.25);

        QVERIFY(panel.width() > 0);
    }
};
