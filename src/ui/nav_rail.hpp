#pragma once

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <vector>

namespace ionexa::ui {

enum class NavSection {
    Workspace,
    Projects,
    Presets,
    Diagnostics,
    Settings
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
    QPushButton* m_btnWorkspace{nullptr};
    QPushButton* m_btnProjects{nullptr};
    QPushButton* m_btnPresets{nullptr};
    QPushButton* m_btnDiagnostics{nullptr};
    QPushButton* m_btnSettings{nullptr};
};

} // namespace ionexa::ui
