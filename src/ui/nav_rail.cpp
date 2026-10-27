#include "nav_rail.hpp"
#include "theme.hpp"

namespace ionexa::ui {

NavRail::NavRail(QWidget* parent)
    : QWidget(parent) {
    setFixedWidth(52);
    setObjectName("navRail");
    setupButtons();
}

void NavRail::setupButtons() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 12, 4, 12);
    layout->setSpacing(8);
    layout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    auto createNavBtn = [this](const QString& text, const QString& tooltip) {
        auto* btn = new QPushButton(text, this);
        btn->setToolTip(tooltip);
        btn->setFixedSize(42, 42);
        btn->setCheckable(true);
        return btn;
    };

    m_btnHome = createNavBtn("⌂", "Home & Workbench Overview (Alt+1)");
    m_btnProjects = createNavBtn("📁", "Project Metadata & Web Packages (Alt+2)");
    m_btnDevices = createNavBtn("📱", "Device Manager & Standalone Emulator (Alt+3)");
    m_btnConsole = createNavBtn("💻", "Developer Console & Logs (Alt+4)");
    m_btnNetwork = createNavBtn("🌐", "Network Inspector & Traffic (Alt+5)");
    m_btnCompatibility = createNavBtn("📊", "Web Compatibility & Native Handoff (Alt+6)");
    m_btnSettings = createNavBtn("⚙", "Settings & Preferences (Alt+7)");

    connect(m_btnHome, &QPushButton::clicked, this, [this]() { setActiveSection(NavSection::Home); });
    connect(m_btnProjects, &QPushButton::clicked, this, [this]() { setActiveSection(NavSection::Projects); });
    connect(m_btnDevices, &QPushButton::clicked, this, [this]() { setActiveSection(NavSection::Devices); });
    connect(m_btnConsole, &QPushButton::clicked, this, [this]() { setActiveSection(NavSection::Console); });
    connect(m_btnNetwork, &QPushButton::clicked, this, [this]() { setActiveSection(NavSection::Network); });
    connect(m_btnCompatibility, &QPushButton::clicked, this, [this]() { setActiveSection(NavSection::Compatibility); });
    connect(m_btnSettings, &QPushButton::clicked, this, [this]() { setActiveSection(NavSection::Settings); });

    layout->addWidget(m_btnHome);
    layout->addWidget(m_btnProjects);
    layout->addWidget(m_btnDevices);
    layout->addWidget(m_btnConsole);
    layout->addWidget(m_btnNetwork);
    layout->addWidget(m_btnCompatibility);
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
    m_btnHome->setChecked(m_activeSection == NavSection::Home);
    m_btnProjects->setChecked(m_activeSection == NavSection::Projects);
    m_btnDevices->setChecked(m_activeSection == NavSection::Devices);
    m_btnConsole->setChecked(m_activeSection == NavSection::Console);
    m_btnNetwork->setChecked(m_activeSection == NavSection::Network);
    m_btnCompatibility->setChecked(m_activeSection == NavSection::Compatibility);
    m_btnSettings->setChecked(m_activeSection == NavSection::Settings);
}

} // namespace ionexa::ui
