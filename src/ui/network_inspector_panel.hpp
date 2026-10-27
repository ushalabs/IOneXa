#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QTabWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QSplitter>
#include "engines/network_inspector.hpp"

namespace ionexa::ui {

class NetworkInspectorPanel : public QWidget {
    Q_OBJECT

public:
    explicit NetworkInspectorPanel(engines::NetworkInspector* inspector, QWidget* parent = nullptr);
    ~NetworkInspectorPanel() override = default;

public slots:
    void refreshTable();

private slots:
    void onRequestStarted(const engines::NetworkRequestEntry& req);
    void onRequestCompleted(const engines::NetworkRequestEntry& req);
    void onRequestFailed(const engines::NetworkRequestEntry& req);
    void onTableRowSelected();
    void onClearClicked();
    void onExportHarClicked();
    void onCopyJsonClicked();
    void onFilterChanged();

private:
    void setupUi();
    void updateSummary();
    void displayRequestDetails(const engines::NetworkRequestEntry& req);

    engines::NetworkInspector* m_inspector{nullptr};

    // UI elements
    QLineEdit* m_searchBox{nullptr};
    QComboBox* m_typeCombo{nullptr};
    QComboBox* m_statusCombo{nullptr};
    QLabel* m_summaryLabel{nullptr};
    QPushButton* m_btnClear{nullptr};
    QPushButton* m_btnExportHar{nullptr};
    QPushButton* m_btnCopyJson{nullptr};

    QTableWidget* m_table{nullptr};
    QTabWidget* m_detailsTabs{nullptr};
    QTextEdit* m_headersView{nullptr};
    QTextEdit* m_responseView{nullptr};
};

} // namespace ionexa::ui
