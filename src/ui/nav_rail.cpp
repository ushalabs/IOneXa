#include "nav_rail.hpp"

namespace ionexa::ui {

NavRail::NavRail(QWidget* parent)
    : QWidget(parent) {
    setFixedWidth(64);
    setObjectName("navRail");
    setupButtons();
}

void NavRail::setupButtons() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 16, 8, 16);
    layout->setSpacing(12);

    m_btnWorkspace = new QPushButton("📱", this);
    m_btnWorkspace->setToolTip("Workspace (Live Preview)");
    m_btnWorkspace->setFixedSize(48, 48);

    m_btnProjects = new QPushButton("📁", this);
    m_btnProjects->setToolTip("Projects & Recent");
    m_btnProjects->setFixedSize(48, 48);

    m_btnPresets = new QPushButton("📐", this);
    m_btnPresets->setToolTip("Device Presets");
    m_btnPresets->setFixedSize(48, 48);

    m_btnDiagnostics = new QPushButton("🔍", this);
    m_btnDiagnostics->setToolTip("Diagnostics & Logs");
    m_btnDiagnostics->setFixedSize(48, 48);

    m_btnSettings = new QPushButton("⚙️", this);
    m_btnSettings->setToolTip("Settings & Preferences");
    m_btnSettings->setFixedSize(48, 48);

    connect(m_btnWorkspace, &QPushButton::clicked, this, [this]() { setActiveSection(NavSection::Workspace); });
    connect(m_btnProjects, &QPushButton::clicked, this, [this]() { setActiveSection(NavSection::Projects); });
    connect(m_btnPresets, &QPushButton::clicked, this, [this]() { setActiveSection(NavSection::Presets); });
    connect(m_btnDiagnostics, &QPushButton::clicked, this, [this]() { setActiveSection(NavSection::Diagnostics); });
    connect(m_btnSettings, &QPushButton::clicked, this, [this]() { setActiveSection(NavSection::Settings); });

    layout->addWidget(m_btnWorkspace);
    layout->addWidget(m_btnProjects);
    layout->addWidget(m_btnPresets);
    layout->addWidget(m_btnDiagnostics);
    layout->addStretch();
    layout->addWidget(m_btnSettings);

    updateButtonStyles();
}

void NavRail::setActiveSection(NavSection section) {
    if (m_activeSection != section) {
        m_activeSection = section;
        updateButtonStyles();
        emit sectionSelected(m_activeSection);
    }
}

NavSection NavRail::activeSection() const {
    return m_activeSection;
}

void NavRail::updateButtonStyles() {
    auto applyStyle = [](QPushButton* btn, bool active) {
        if (!btn) return;
        if (active) {
            btn->setStyleSheet(
                "QPushButton {"
                "  background-color: #0284c7;"
                "  border: 2px solid #38bdf8;"
                "  border-radius: 8px;"
                "  font-size: 20px;"
                "}"
            );
        } else {
            btn->setStyleSheet(
                "QPushButton {"
                "  background-color: #1e293b;"
                "  border: 1px solid #334155;"
                "  border-radius: 8px;"
                "  font-size: 20px;"
                "}"
                "QPushButton:hover {"
                "  background-color: #334155;"
                "  border-color: #475569;"
                "}"
            );
        }
    };

    applyStyle(m_btnWorkspace, m_activeSection == NavSection::Workspace);
    applyStyle(m_btnProjects, m_activeSection == NavSection::Projects);
    applyStyle(m_btnPresets, m_activeSection == NavSection::Presets);
    applyStyle(m_btnDiagnostics, m_activeSection == NavSection::Diagnostics);
    applyStyle(m_btnSettings, m_activeSection == NavSection::Settings);
}

} // namespace ionexa::ui
