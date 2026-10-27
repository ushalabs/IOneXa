#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QVector>

#include "viewport_manager.hpp"

namespace ionexa::ui {

class DeviceManagerPanel : public QWidget {
    Q_OBJECT

public:
    explicit DeviceManagerPanel(ViewportManager* viewportManager, QWidget* parent = nullptr);
    ~DeviceManagerPanel() override = default;

    void refreshCatalog();

signals:
    void launchRequested(const QString& profileId);

private slots:
    void onAddCustomProfileClicked();
    void onResetDefaultsClicked();
    void onFilterChanged(const QString& text);

private:
    void setupUi();
    QWidget* createProfileCard(const DeviceProfile& prof, int index);

    ViewportManager* m_viewportManager{nullptr};
    QLineEdit* m_searchBox{nullptr};
    QVBoxLayout* m_cardsLayout{nullptr};
    QLabel* m_statusLabel{nullptr};
};

} // namespace ionexa::ui
