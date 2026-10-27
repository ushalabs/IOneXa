#pragma once

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include "project/project_scanner.hpp"
#include "process/metro_supervisor.hpp"

namespace ionexa::ui {

class InspectorPanel : public QWidget {
    Q_OBJECT

public:
    explicit InspectorPanel(QWidget* parent = nullptr);

    void updateProject(const project::ProjectDetails& details);
    void updateProcess(int pid, int port, process::MetroStatus status);
    void updateViewport(int width, int height, bool isLandscape, double zoom);

private:
    void setupUi();

    // Section 1: Engine
    QLabel* m_lblEngineName{nullptr};
    QLabel* m_lblFidelityDisclosure{nullptr};

    // Section 2: Viewport
    QLabel* m_lblDimensions{nullptr};
    QLabel* m_lblOrientation{nullptr};
    QLabel* m_lblZoom{nullptr};

    // Section 3: Process Diagnostics
    QLabel* m_lblProcessStatus{nullptr};
    QLabel* m_lblProcessPid{nullptr};
    QLabel* m_lblProcessPort{nullptr};

    // Section 4: Project Metadata
    QLabel* m_lblProjectName{nullptr};
    QLabel* m_lblExpoSdk{nullptr};
    QLabel* m_lblPackageManager{nullptr};
    QLabel* m_lblWebSupport{nullptr};
};

} // namespace ionexa::ui
