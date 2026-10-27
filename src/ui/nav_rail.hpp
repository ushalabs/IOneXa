#pragma once

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <vector>

namespace ionexa::ui {

enum class NavSection {
    Workspace = 0,
    Home = 0,
    Projects = 1,
    Presets = 2,
    Devices = 2,
    Diagnostics = 3,
    Console = 3,
    Network = 4,
    Compatibility = 5,
    Settings = 6
};

class NavRail : public QWidget {
    Q_OBJECT

public:
    explicit NavRail(QWidget* parent = nullptr);

    void setActiveSection(NavSection section);
    [[nodiscard]] NavSection activeSection() const;

signals:
    void sectionSelected(NavSection section);

private:
    void setupButtons();
    void updateButtonStyles();

    NavSection m_activeSection{NavSection::Workspace};
    QPushButton* m_btnHome{nullptr};
    QPushButton* m_btnProjects{nullptr};
    QPushButton* m_btnDevices{nullptr};
    QPushButton* m_btnConsole{nullptr};
    QPushButton* m_btnNetwork{nullptr};
    QPushButton* m_btnCompatibility{nullptr};
    QPushButton* m_btnSettings{nullptr};
};

} // namespace ionexa::ui
