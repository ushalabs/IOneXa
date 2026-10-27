#include "device_manager_panel.hpp"
#include "theme.hpp"

#include <QDateTime>

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QMessageBox>
#include <QSpinBox>

namespace ionexa::ui {

DeviceManagerPanel::DeviceManagerPanel(ViewportManager* viewportManager, QWidget* parent)
    : QWidget(parent)
    , m_viewportManager(viewportManager) {
    setupUi();

    if (m_viewportManager) {
        connect(m_viewportManager, &ViewportManager::profileListChanged, this, &DeviceManagerPanel::refreshCatalog);
        connect(m_viewportManager, &ViewportManager::profileChanged, this, [this](const DeviceProfile&) {
            refreshCatalog();
        });
    }

    refreshCatalog();
}

void DeviceManagerPanel::setupUi() {
    setStyleSheet("background-color: #16181D;");

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(24, 24, 24, 24);
    rootLayout->setSpacing(16);

    // Header Bar
    auto* headerLayout = new QHBoxLayout();
    auto* title = new QLabel("Device Profiles & Viewport Manager", this);
    title->setStyleSheet("font-size: 18px; font-weight: 700; color: #F2F4F8;");
    headerLayout->addWidget(title);

    headerLayout->addStretch();

    auto* btnAdd = new QPushButton("➕ Add Custom Device", this);
    btnAdd->setStyleSheet(Theme::buttonStyle(true));
    connect(btnAdd, &QPushButton::clicked, this, &DeviceManagerPanel::onAddCustomProfileClicked);
    headerLayout->addWidget(btnAdd);

    auto* btnReset = new QPushButton("Reset to Defaults", this);
    btnReset->setStyleSheet(Theme::buttonStyle(false));
    connect(btnReset, &QPushButton::clicked, this, &DeviceManagerPanel::onResetDefaultsClicked);
    headerLayout->addWidget(btnReset);

    rootLayout->addLayout(headerLayout);

    // Search and Filter Bar
    auto* searchBar = new QHBoxLayout();
    m_searchBox = new QLineEdit(this);
    m_searchBox->setPlaceholderText("Search device profiles by name or dimensions...");
    m_searchBox->setStyleSheet(Theme::lineEditStyle());
    connect(m_searchBox, &QLineEdit::textChanged, this, &DeviceManagerPanel::onFilterChanged);
    searchBar->addWidget(m_searchBox);

    rootLayout->addLayout(searchBar);

    // Scroll Area for Profile Cards
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("QScrollArea { border: none; background-color: #16181D; }");

    auto* container = new QWidget();
    container->setStyleSheet("background-color: #16181D;");
    m_cardsLayout = new QVBoxLayout(container);
    m_cardsLayout->setContentsMargins(0, 0, 0, 0);
    m_cardsLayout->setSpacing(12);

    scroll->setWidget(container);
    rootLayout->addWidget(scroll, 1);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet("color: #6B7688; font-size: 11px;");
    rootLayout->addWidget(m_statusLabel);
}

void DeviceManagerPanel::refreshCatalog() {
    if (!m_cardsLayout || !m_viewportManager) return;

    // Clear existing cards
    QLayoutItem* item = nullptr;
    while ((item = m_cardsLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    QString filter = m_searchBox ? m_searchBox->text().trimmed().toLower() : QString();
    const auto& profiles = m_viewportManager->profiles();
    int shown = 0;

    for (int i = 0; i < profiles.size(); ++i) {
        const auto& prof = profiles[i];
        if (!filter.isEmpty()) {
            QString haystack = QString("%1 %2 %3").arg(prof.name).arg(prof.width).arg(prof.height).toLower();
            if (!haystack.contains(filter)) continue;
        }

        m_cardsLayout->addWidget(createProfileCard(prof, i));
        shown++;
    }

    m_cardsLayout->addStretch();
    if (m_statusLabel) {
        m_statusLabel->setText(QString("Showing %1 of %2 device profiles | Active: %3")
            .arg(shown).arg(profiles.size()).arg(m_viewportManager->activeProfile().name));
    }
}

QWidget* DeviceManagerPanel::createProfileCard(const DeviceProfile& prof, int index) {
    auto* card = new QFrame();
    card->setObjectName("cardFrame");
    card->setStyleSheet(Theme::cardStyle());
    auto* cardLayout = new QHBoxLayout(card);
    cardLayout->setContentsMargins(18, 14, 18, 14);
    cardLayout->setSpacing(14);

    // Left Device Silhouette / Icon
    auto* iconBox = new QFrame(card);
    iconBox->setFixedSize(48, 48);
    iconBox->setStyleSheet("background-color: #1E222A; border: 1px solid #343B48; border-radius: 8px;");
    auto* iconLayout = new QVBoxLayout(iconBox);
    iconLayout->setContentsMargins(0, 0, 0, 0);
    auto* iconLabel = new QLabel(prof.isMobile ? "📱" : "💻", iconBox);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet("font-size: 22px;");
    iconLayout->addWidget(iconLabel);
    cardLayout->addWidget(iconBox);

    // Middle Info Block
    auto* infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(4);

    auto* nameLayout = new QHBoxLayout();
    auto* nameLabel = new QLabel(prof.name, card);
    nameLabel->setStyleSheet("font-size: 14px; font-weight: 700; color: #F2F4F8;");
    nameLayout->addWidget(nameLabel);

    if (index == m_viewportManager->activeProfileIndex()) {
        auto* activeBadge = new QLabel("ACTIVE VIEWPORT", card);
        activeBadge->setStyleSheet(Theme::badgeStyle("#43C990", "#000000"));
        nameLayout->addWidget(activeBadge);
    }

    if (prof.hasDynamicIsland) {
        auto* diBadge = new QLabel("Dynamic Island", card);
        diBadge->setStyleSheet(Theme::badgeStyle("#1E222A", "#4E9CFF"));
        nameLayout->addWidget(diBadge);
    }

    nameLayout->addStretch();
    infoLayout->addLayout(nameLayout);

    QString meta = QString("Viewport: %1 x %2 px | DPI Scale: %3x | %4")
        .arg(prof.width).arg(prof.height).arg(prof.scaleFactor)
        .arg(prof.isMobile ? "Mobile Device" : "Tablet / Desktop");
    auto* metaLabel = new QLabel(meta, card);
    metaLabel->setStyleSheet("color: #A4ADBD; font-size: 12px;");
    infoLayout->addWidget(metaLabel);

    cardLayout->addLayout(infoLayout, 1);

    // Right Action Buttons
    auto* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(8);

    auto* btnSelect = new QPushButton("📱 Launch Emulator", card);
    btnSelect->setStyleSheet(Theme::buttonStyle(index == m_viewportManager->activeProfileIndex()));
    connect(btnSelect, &QPushButton::clicked, this, [this, index, prof]() {
        m_viewportManager->selectProfile(index);
        emit launchRequested(prof.id);
    });
    btnLayout->addWidget(btnSelect);

    auto* btnDuplicate = new QPushButton("📑 Duplicate", card);
    btnDuplicate->setStyleSheet(Theme::buttonStyle(false));
    connect(btnDuplicate, &QPushButton::clicked, this, [this, prof]() {
        m_viewportManager->duplicateProfile(prof.id, QString("%1 (Custom)").arg(prof.name));
    });
    btnLayout->addWidget(btnDuplicate);

    // Only allow deletion for custom or duplicated profiles
    bool isBuiltin = (prof.id == "iphone-14" || prof.id == "iphone-15-pro-max" || prof.id == "iphone-se" || prof.id == "pixel-7" || prof.id == "galaxy-s23" || prof.id == "ipad-mini");
    if (!isBuiltin) {
        auto* btnDelete = new QPushButton("🗑 Delete", card);
        btnDelete->setStyleSheet(Theme::buttonStyle(false, true));
        connect(btnDelete, &QPushButton::clicked, this, [this, prof]() {
            m_viewportManager->removeProfile(prof.id);
        });
        btnLayout->addWidget(btnDelete);
    }

    cardLayout->addLayout(btnLayout);
    return card;
}

void DeviceManagerPanel::onAddCustomProfileClicked() {
    QDialog dialog(this);
    dialog.setWindowTitle("Create Custom Device Profile");
    dialog.setStyleSheet(Theme::workbenchStyle());
    dialog.setMinimumWidth(380);

    auto* layout = new QVBoxLayout(&dialog);
    layout->setSpacing(12);

    auto* formLayout = new QFormLayout();
    formLayout->setSpacing(10);

    auto* nameEdit = new QLineEdit("Custom Phone", &dialog);
    nameEdit->setStyleSheet(Theme::lineEditStyle());
    formLayout->addRow("Profile Name:", nameEdit);

    auto* widthSpin = new QSpinBox(&dialog);
    widthSpin->setRange(200, 3840);
    widthSpin->setValue(393);
    formLayout->addRow("Width (px):", widthSpin);

    auto* heightSpin = new QSpinBox(&dialog);
    heightSpin->setRange(200, 3840);
    heightSpin->setValue(852);
    formLayout->addRow("Height (px):", heightSpin);

    auto* scaleSpin = new QDoubleSpinBox(&dialog);
    scaleSpin->setRange(1.0, 4.0);
    scaleSpin->setSingleStep(0.25);
    scaleSpin->setValue(3.0);
    formLayout->addRow("Scale Factor:", scaleSpin);

    auto* diCheck = new QCheckBox("Simulate Dynamic Island", &dialog);
    formLayout->addRow("", diCheck);

    auto* mobileCheck = new QCheckBox("Mobile Viewport Emulation", &dialog);
    mobileCheck->setChecked(true);
    formLayout->addRow("", mobileCheck);

    layout->addLayout(formLayout);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);

    if (dialog.exec() == QDialog::Accepted) {
        DeviceProfile prof;
        prof.id = QString("custom-%1").arg(QDateTime::currentMSecsSinceEpoch());
        prof.name = nameEdit->text().trimmed();
        if (prof.name.isEmpty()) prof.name = "Custom Device";
        prof.width = widthSpin->value();
        prof.height = heightSpin->value();
        prof.scaleFactor = scaleSpin->value();
        prof.hasDynamicIsland = diCheck->isChecked();
        prof.isMobile = mobileCheck->isChecked();
        prof.userAgent = "Mozilla/5.0 (iPhone; CPU iPhone OS 17_4 like Mac OS X) AppleWebKit/605.1.15";

        m_viewportManager->addCustomProfile(prof);
    }
}

void DeviceManagerPanel::onResetDefaultsClicked() {
    auto reply = QMessageBox::question(
        this,
        "Reset Device Profiles",
        "Are you sure you want to restore the default standard device catalog?",
        QMessageBox::Yes | QMessageBox::No
    );
    if (reply == QMessageBox::Yes) {
        m_viewportManager->resetToDefaultProfiles();
    }
}

void DeviceManagerPanel::onFilterChanged(const QString& text) {
    Q_UNUSED(text);
    refreshCatalog();
}

} // namespace ionexa::ui
