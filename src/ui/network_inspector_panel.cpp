#include "network_inspector_panel.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QClipboard>
#include <QApplication>
#include <QFileDialog>
#include <QFile>

namespace ionexa::ui {

NetworkInspectorPanel::NetworkInspectorPanel(engines::NetworkInspector* inspector, QWidget* parent)
    : QWidget(parent), m_inspector(inspector) {
    setupUi();

    if (m_inspector) {
        connect(m_inspector, &engines::NetworkInspector::requestStarted, this, &NetworkInspectorPanel::onRequestStarted);
        connect(m_inspector, &engines::NetworkInspector::requestCompleted, this, &NetworkInspectorPanel::onRequestCompleted);
        connect(m_inspector, &engines::NetworkInspector::requestFailed, this, &NetworkInspectorPanel::onRequestFailed);
        connect(m_inspector, &engines::NetworkInspector::cleared, this, &NetworkInspectorPanel::refreshTable);
    }
}

void NetworkInspectorPanel::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Header Toolbar
    auto* toolbar = new QWidget(this);
    toolbar->setFixedHeight(38);
    toolbar->setStyleSheet("background-color: #0f172a; border-bottom: 1px solid #1e293b;");
    auto* tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(10, 4, 10, 4);
    tbLayout->setSpacing(8);

    auto* lblTitle = new QLabel("NETWORK INSPECTOR", toolbar);
    lblTitle->setStyleSheet("font-weight: 700; font-size: 11px; color: #94a3b8;");
    tbLayout->addWidget(lblTitle);

    m_searchBox = new QLineEdit(toolbar);
    m_searchBox->setPlaceholderText("Filter URL...");
    m_searchBox->setFixedWidth(160);
    m_searchBox->setStyleSheet("background-color: #020617; color: #f8fafc; border: 1px solid #334155; border-radius: 4px; padding: 2px 6px; font-size: 11px;");
    connect(m_searchBox, &QLineEdit::textChanged, this, &NetworkInspectorPanel::onFilterChanged);
    tbLayout->addWidget(m_searchBox);

    m_typeCombo = new QComboBox(toolbar);
    m_typeCombo->addItem("All Types", static_cast<int>(engines::ResourceType::All));
    m_typeCombo->addItem("Fetch / XHR", static_cast<int>(engines::ResourceType::Fetch));
    m_typeCombo->addItem("Scripts (JS)", static_cast<int>(engines::ResourceType::Script));
    m_typeCombo->addItem("Stylesheets (CSS)", static_cast<int>(engines::ResourceType::Stylesheet));
    m_typeCombo->addItem("Images", static_cast<int>(engines::ResourceType::Image));
    m_typeCombo->setStyleSheet("background-color: #020617; color: #f8fafc; border: 1px solid #334155; border-radius: 4px; padding: 2px 6px; font-size: 11px;");
    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &NetworkInspectorPanel::onFilterChanged);
    tbLayout->addWidget(m_typeCombo);

    m_statusCombo = new QComboBox(toolbar);
    m_statusCombo->addItem("All Status", 0);
    m_statusCombo->addItem("2xx Success", 200);
    m_statusCombo->addItem("4xx Client Error", 400);
    m_statusCombo->addItem("5xx Server Error", 500);
    m_statusCombo->setStyleSheet("background-color: #020617; color: #f8fafc; border: 1px solid #334155; border-radius: 4px; padding: 2px 6px; font-size: 11px;");
    connect(m_statusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &NetworkInspectorPanel::onFilterChanged);
    tbLayout->addWidget(m_statusCombo);

    m_summaryLabel = new QLabel("0 requests | 0 B", toolbar);
    m_summaryLabel->setStyleSheet("color: #38bdf8; font-size: 11px; font-weight: 600;");
    tbLayout->addWidget(m_summaryLabel);

    tbLayout->addStretch();

    m_btnCopyJson = new QPushButton("Copy JSON", toolbar);
    m_btnCopyJson->setStyleSheet("background-color: #1e293b; color: #f8fafc; border: 1px solid #334155; border-radius: 4px; padding: 3px 10px; font-size: 11px;");
    connect(m_btnCopyJson, &QPushButton::clicked, this, &NetworkInspectorPanel::onCopyJsonClicked);
    tbLayout->addWidget(m_btnCopyJson);

    m_btnExportHar = new QPushButton("Export HAR", toolbar);
    m_btnExportHar->setStyleSheet("background-color: #1e293b; color: #f8fafc; border: 1px solid #334155; border-radius: 4px; padding: 3px 10px; font-size: 11px;");
    connect(m_btnExportHar, &QPushButton::clicked, this, &NetworkInspectorPanel::onExportHarClicked);
    tbLayout->addWidget(m_btnExportHar);

    m_btnClear = new QPushButton("Clear", toolbar);
    m_btnClear->setStyleSheet("background-color: #1e293b; color: #f8fafc; border: 1px solid #334155; border-radius: 4px; padding: 3px 10px; font-size: 11px;");
    connect(m_btnClear, &QPushButton::clicked, this, &NetworkInspectorPanel::onClearClicked);
    tbLayout->addWidget(m_btnClear);

    mainLayout->addWidget(toolbar);

    // Splitter: Table (Left) | Details (Right)
    auto* splitter = new QSplitter(Qt::Horizontal, this);

    // Table
    m_table = new QTableWidget(splitter);
    m_table->setColumnCount(6);
    m_table->setHorizontalHeaderLabels({"Status", "Method", "URL", "Type", "Size", "Time"});
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->verticalHeader()->setVisible(false);
    m_table->setStyleSheet(R"(
        QTableWidget {
            background-color: #020617;
            color: #cbd5e1;
            gridline-color: #1e293b;
            border: none;
            font-size: 11px;
        }
        QTableWidget::item:selected {
            background-color: #1e3a8a;
            color: #ffffff;
        }
        QHeaderView::section {
            background-color: #0f172a;
            color: #94a3b8;
            font-weight: 600;
            padding: 4px;
            border: 1px solid #1e293b;
        }
    )");
    connect(m_table, &QTableWidget::itemSelectionChanged, this, &NetworkInspectorPanel::onTableRowSelected);
    splitter->addWidget(m_table);

    // Details Pane
    m_detailsTabs = new QTabWidget(splitter);
    m_detailsTabs->setStyleSheet(R"(
        QTabWidget::pane {
            border: 1px solid #1e293b;
            background-color: #0b0f19;
        }
        QTabBar::tab {
            background-color: #0f172a;
            color: #94a3b8;
            padding: 6px 12px;
            font-size: 11px;
            font-weight: 600;
        }
        QTabBar::tab:selected {
            background-color: #1e293b;
            color: #38bdf8;
        }
    )");

    m_headersView = new QTextEdit(m_detailsTabs);
    m_headersView->setReadOnly(true);
    m_headersView->setStyleSheet("background-color: #020617; color: #cbd5e1; border: none; font-family: monospace; font-size: 11px; padding: 6px;");
    m_detailsTabs->addTab(m_headersView, "Headers");

    m_responseView = new QTextEdit(m_detailsTabs);
    m_responseView->setReadOnly(true);
    m_responseView->setStyleSheet("background-color: #020617; color: #cbd5e1; border: none; font-family: monospace; font-size: 11px; padding: 6px;");
    m_detailsTabs->addTab(m_responseView, "Response");

    splitter->addWidget(m_detailsTabs);
    splitter->setSizes({600, 300});

    mainLayout->addWidget(splitter, 1);
}

void NetworkInspectorPanel::refreshTable() {
    if (!m_inspector) return;

    auto typeFilter = static_cast<engines::ResourceType>(m_typeCombo->currentData().toInt());
    int statusFilter = m_statusCombo->currentData().toInt();
    QString search = m_searchBox->text().trimmed();

    auto filtered = m_inspector->filteredRequests(typeFilter, statusFilter, search);

    m_table->setRowCount(0);
    for (const auto& req : filtered) {
        int row = m_table->rowCount();
        m_table->insertRow(row);

        // Status
        QString statusText = req.isCompleted ? (req.isFailed ? QString("%1 (Fail)").arg(req.statusCode) : QString::number(req.statusCode)) : "...";
        auto* itemStatus = new QTableWidgetItem(statusText);
        itemStatus->setData(Qt::UserRole, req.id);
        if (req.statusCode >= 200 && req.statusCode < 300) {
            itemStatus->setForeground(QColor("#4ade80"));
        } else if (req.statusCode >= 300 && req.statusCode < 400) {
            itemStatus->setForeground(QColor("#38bdf8"));
        } else if (req.statusCode >= 400) {
            itemStatus->setForeground(QColor("#f87171"));
        } else {
            itemStatus->setForeground(QColor("#fde047"));
        }
        m_table->setItem(row, 0, itemStatus);

        // Method
        auto* itemMethod = new QTableWidgetItem(req.method);
        itemMethod->setForeground(QColor("#c084fc"));
        m_table->setItem(row, 1, itemMethod);

        // URL
        auto* itemUrl = new QTableWidgetItem(req.url);
        itemUrl->setToolTip(req.url);
        m_table->setItem(row, 2, itemUrl);

        // Type
        QString typeStr = "Fetch";
        switch (req.resourceType) {
        case engines::ResourceType::Script: typeStr = "Script"; break;
        case engines::ResourceType::Stylesheet: typeStr = "CSS"; break;
        case engines::ResourceType::Image: typeStr = "Img"; break;
        case engines::ResourceType::Font: typeStr = "Font"; break;
        case engines::ResourceType::Document: typeStr = "Doc"; break;
        case engines::ResourceType::XHR: typeStr = "XHR"; break;
        default: break;
        }
        m_table->setItem(row, 3, new QTableWidgetItem(typeStr));

        // Size
        QString sizeStr = req.responseBytes > 1024 ? QString("%1 KB").arg(req.responseBytes / 1024.0, 0, 'f', 1) : QString("%1 B").arg(req.responseBytes);
        m_table->setItem(row, 4, new QTableWidgetItem(sizeStr));

        // Time
        QString timeStr = QString("%1 ms").arg(req.durationMs);
        m_table->setItem(row, 5, new QTableWidgetItem(timeStr));
    }

    updateSummary();
}

void NetworkInspectorPanel::updateSummary() {
    if (!m_inspector) return;
    qint64 bytes = m_inspector->totalTransferredBytes();
    QString sizeStr = bytes > 1024 * 1024 ? QString("%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 2) :
                      bytes > 1024 ? QString("%1 KB").arg(bytes / 1024.0, 0, 'f', 1) : QString("%1 B").arg(bytes);
    int errors = m_inspector->errorCount();
    m_summaryLabel->setText(QString("%1 requests | %2 | %3 errors").arg(m_inspector->totalCount()).arg(sizeStr).arg(errors));
}

void NetworkInspectorPanel::onRequestStarted(const engines::NetworkRequestEntry& req) {
    Q_UNUSED(req);
    refreshTable();
}

void NetworkInspectorPanel::onRequestCompleted(const engines::NetworkRequestEntry& req) {
    Q_UNUSED(req);
    refreshTable();
}

void NetworkInspectorPanel::onRequestFailed(const engines::NetworkRequestEntry& req) {
    Q_UNUSED(req);
    refreshTable();
}

void NetworkInspectorPanel::onFilterChanged() {
    refreshTable();
}

void NetworkInspectorPanel::onTableRowSelected() {
    auto selectedItems = m_table->selectedItems();
    if (selectedItems.isEmpty()) {
        m_headersView->clear();
        m_responseView->clear();
        return;
    }

    int row = selectedItems.first()->row();
    auto* statusItem = m_table->item(row, 0);
    if (!statusItem) return;

    qint64 reqId = statusItem->data(Qt::UserRole).toLongLong();
    for (const auto& req : m_inspector->allRequests()) {
        if (req.id == reqId) {
            displayRequestDetails(req);
            break;
        }
    }
}

void NetworkInspectorPanel::displayRequestDetails(const engines::NetworkRequestEntry& req) {
    // 1. Headers View
    QString html = "<h3>General</h3>";
    html += QString("<b>Request URL:</b> %1<br/>").arg(req.url.toHtmlEscaped());
    html += QString("<b>Request Method:</b> %1<br/>").arg(req.method);
    html += QString("<b>Status Code:</b> %1 %2<br/>").arg(req.statusCode).arg(req.statusText.toHtmlEscaped());
    html += QString("<b>Duration:</b> %1 ms<br/>").arg(req.durationMs);

    html += "<h3>Request Headers</h3>";
    if (req.requestHeaders.isEmpty()) {
        html += "<i>None</i><br/>";
    } else {
        for (auto it = req.requestHeaders.begin(); it != req.requestHeaders.end(); ++it) {
            html += QString("<b>%1:</b> %2<br/>").arg(it.key().toHtmlEscaped()).arg(it.value().toHtmlEscaped());
        }
    }

    html += "<h3>Response Headers</h3>";
    if (req.responseHeaders.isEmpty()) {
        html += "<i>None</i><br/>";
    } else {
        for (auto it = req.responseHeaders.begin(); it != req.responseHeaders.end(); ++it) {
            html += QString("<b>%1:</b> %2<br/>").arg(it.key().toHtmlEscaped()).arg(it.value().toHtmlEscaped());
        }
    }

    m_headersView->setHtml(html);

    // 2. Response View
    if (!req.responseBody.isEmpty()) {
        m_responseView->setPlainText(req.responseBody);
    } else if (req.isFailed && !req.failureReason.isEmpty()) {
        m_responseView->setPlainText(QString("Request Failed: %1").arg(req.failureReason));
    } else {
        m_responseView->setPlainText("(No response body preview available)");
    }
}

void NetworkInspectorPanel::onClearClicked() {
    if (m_inspector) {
        m_inspector->clear();
    }
    m_headersView->clear();
    m_responseView->clear();
}

void NetworkInspectorPanel::onExportHarClicked() {
    if (!m_inspector) return;
    QString path = QFileDialog::getSaveFileName(this, "Export HTTP Archive", "session.har", "HAR Files (*.har);;JSON Files (*.json)");
    if (!path.isEmpty()) {
        QFile file(path);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write(m_inspector->exportAsHar().toUtf8());
        }
    }
}

void NetworkInspectorPanel::onCopyJsonClicked() {
    if (!m_inspector) return;
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(m_inspector->exportAsJson());
}

} // namespace ionexa::ui
