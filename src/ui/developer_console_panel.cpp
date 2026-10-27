#include "developer_console_panel.hpp"

#include <QClipboard>
#include <QApplication>
#include <QFileDialog>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace ionexa::ui {

DeveloperConsolePanel::DeveloperConsolePanel(QWidget* parent)
    : QWidget(parent) {
    setupUi();
}

void DeveloperConsolePanel::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Header Toolbar
    auto* toolbar = new QWidget(this);
    toolbar->setObjectName("consoleToolbar");
    toolbar->setFixedHeight(38);
    auto* tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(10, 4, 10, 4);
    tbLayout->setSpacing(8);

    auto* lblTitle = new QLabel("CONSOLE & DIAGNOSTICS", toolbar);
    lblTitle->setStyleSheet("font-weight: 700; font-size: 11px; color: #94a3b8;");
    tbLayout->addWidget(lblTitle);

    // Counter Badges
    m_badgeErrors = new QLabel("0 Errors", toolbar);
    m_badgeErrors->setStyleSheet("background-color: #450a0a; color: #f87171; border: 1px solid #7f1d1d; border-radius: 4px; padding: 2px 6px; font-size: 11px; font-weight: 600;");
    m_badgeWarnings = new QLabel("0 Warnings", toolbar);
    m_badgeWarnings->setStyleSheet("background-color: #451a03; color: #fbbf24; border: 1px solid #78350f; border-radius: 4px; padding: 2px 6px; font-size: 11px; font-weight: 600;");
    m_badgeInfo = new QLabel("0 Info", toolbar);
    m_badgeInfo->setStyleSheet("background-color: #0c4a6e; color: #38bdf8; border: 1px solid #0369a1; border-radius: 4px; padding: 2px 6px; font-size: 11px; font-weight: 600;");

    tbLayout->addWidget(m_badgeErrors);
    tbLayout->addWidget(m_badgeWarnings);
    tbLayout->addWidget(m_badgeInfo);

    tbLayout->addSpacing(10);

    // Search Box
    m_searchBox = new QLineEdit(toolbar);
    m_searchBox->setPlaceholderText("Filter logs...");
    m_searchBox->setFixedWidth(180);
    m_searchBox->setStyleSheet("background-color: #0f172a; color: #f8fafc; border: 1px solid #334155; border-radius: 4px; padding: 3px 8px; font-size: 11px;");
    connect(m_searchBox, &QLineEdit::textChanged, this, &DeveloperConsolePanel::onSearchTextChanged);
    tbLayout->addWidget(m_searchBox);

    // Severity Filter Combo
    m_levelCombo = new QComboBox(toolbar);
    m_levelCombo->addItem("All Levels", static_cast<int>(ConsoleLogLevel::All));
    m_levelCombo->addItem("Errors Only", static_cast<int>(ConsoleLogLevel::Error));
    m_levelCombo->addItem("Warnings & Errors", static_cast<int>(ConsoleLogLevel::Warning));
    m_levelCombo->addItem("Info", static_cast<int>(ConsoleLogLevel::Info));
    m_levelCombo->addItem("Debug", static_cast<int>(ConsoleLogLevel::Debug));
    m_levelCombo->setStyleSheet("background-color: #0f172a; color: #f8fafc; border: 1px solid #334155; border-radius: 4px; padding: 2px 6px; font-size: 11px;");
    connect(m_levelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DeveloperConsolePanel::onLevelFilterChanged);
    tbLayout->addWidget(m_levelCombo);

    // Source Filter Combo
    m_sourceCombo = new QComboBox(toolbar);
    m_sourceCombo->addItem("All Sources", static_cast<int>(ConsoleLogSource::All));
    m_sourceCombo->addItem("Metro CLI", static_cast<int>(ConsoleLogSource::Metro));
    m_sourceCombo->addItem("Browser DOM", static_cast<int>(ConsoleLogSource::Browser));
    m_sourceCombo->addItem("System", static_cast<int>(ConsoleLogSource::System));
    m_sourceCombo->setStyleSheet("background-color: #0f172a; color: #f8fafc; border: 1px solid #334155; border-radius: 4px; padding: 2px 6px; font-size: 11px;");
    connect(m_sourceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DeveloperConsolePanel::onSourceFilterChanged);
    tbLayout->addWidget(m_sourceCombo);

    tbLayout->addStretch();

    // Buttons
    m_btnCopy = new QPushButton("Copy", toolbar);
    m_btnCopy->setStyleSheet("background-color: #1e293b; color: #f8fafc; border: 1px solid #334155; border-radius: 4px; padding: 3px 10px; font-size: 11px;");
    connect(m_btnCopy, &QPushButton::clicked, this, &DeveloperConsolePanel::onCopyClicked);
    tbLayout->addWidget(m_btnCopy);

    m_btnExport = new QPushButton("Export", toolbar);
    m_btnExport->setStyleSheet("background-color: #1e293b; color: #f8fafc; border: 1px solid #334155; border-radius: 4px; padding: 3px 10px; font-size: 11px;");
    connect(m_btnExport, &QPushButton::clicked, this, &DeveloperConsolePanel::onExportClicked);
    tbLayout->addWidget(m_btnExport);

    m_btnClear = new QPushButton("Clear", toolbar);
    m_btnClear->setStyleSheet("background-color: #1e293b; color: #f8fafc; border: 1px solid #334155; border-radius: 4px; padding: 3px 10px; font-size: 11px;");
    connect(m_btnClear, &QPushButton::clicked, this, &DeveloperConsolePanel::onClearClicked);
    tbLayout->addWidget(m_btnClear);

    mainLayout->addWidget(toolbar);

    // Log Output Display Area
    m_displayArea = new QTextEdit(this);
    m_displayArea->setReadOnly(true);
    m_displayArea->setObjectName("consoleDisplay");
    m_displayArea->setStyleSheet(R"(
        #consoleDisplay {
            background-color: #020617;
            color: #cbd5e1;
            border: none;
            border-top: 1px solid #1e293b;
            font-family: 'Cascadia Code', 'Consolas', monospace;
            font-size: 12px;
            padding: 8px;
        }
    )");
    mainLayout->addWidget(m_displayArea, 1);
}

void DeveloperConsolePanel::appendLog(ConsoleLogLevel level, ConsoleLogSource source, const QString& message, const QString& stack) {
    ConsoleLogEntry entry;
    entry.id = m_nextId++;
    entry.timestamp = QDateTime::currentDateTime();
    entry.level = level;
    entry.source = source;
    entry.message = message;
    entry.stackTrace = stack;

    m_entries.push_back(entry);
    if (m_entries.size() > m_maxEntries) {
        m_entries.removeFirst();
    }

    updateCounters();

    if (matchesFilter(entry)) {
        m_displayArea->append(formatEntryHtml(entry));
    }

    emit logAdded(entry);
}

void DeveloperConsolePanel::clear() {
    m_entries.clear();
    m_displayArea->clear();
    updateCounters();
    emit consoleCleared();
}

void DeveloperConsolePanel::updateCounters() {
    int errors = countByLevel(ConsoleLogLevel::Error);
    int warnings = countByLevel(ConsoleLogLevel::Warning);
    int info = countByLevel(ConsoleLogLevel::Info);

    m_badgeErrors->setText(QString("%1 Errors").arg(errors));
    m_badgeWarnings->setText(QString("%1 Warnings").arg(warnings));
    m_badgeInfo->setText(QString("%1 Info").arg(info));
}

int DeveloperConsolePanel::countByLevel(ConsoleLogLevel level) const {
    int count = 0;
    for (const auto& entry : m_entries) {
        if (entry.level == level) {
            count++;
        }
    }
    return count;
}

bool DeveloperConsolePanel::matchesFilter(const ConsoleLogEntry& entry) const {
    // Level filter
    if (m_filterLevel == ConsoleLogLevel::Error && entry.level != ConsoleLogLevel::Error) {
        return false;
    }
    if (m_filterLevel == ConsoleLogLevel::Warning && (entry.level != ConsoleLogLevel::Warning && entry.level != ConsoleLogLevel::Error)) {
        return false;
    }
    if (m_filterLevel == ConsoleLogLevel::Info && entry.level != ConsoleLogLevel::Info) {
        return false;
    }
    if (m_filterLevel == ConsoleLogLevel::Debug && entry.level != ConsoleLogLevel::Debug) {
        return false;
    }

    // Source filter
    if (m_filterSource != ConsoleLogSource::All && entry.source != m_filterSource) {
        return false;
    }

    // Substring search
    if (!m_searchText.isEmpty() && !entry.message.contains(m_searchText, Qt::CaseInsensitive)) {
        return false;
    }

    return true;
}

QVector<ConsoleLogEntry> DeveloperConsolePanel::filteredEntries() const {
    QVector<ConsoleLogEntry> result;
    for (const auto& entry : m_entries) {
        if (matchesFilter(entry)) {
            result.push_back(entry);
        }
    }
    return result;
}

QString DeveloperConsolePanel::formatEntryHtml(const ConsoleLogEntry& entry) const {
    QString timeStr = entry.timestamp.toString("hh:mm:ss.zzz");
    QString levelColor = "#94a3b8"; // default Debug
    QString levelTag = "DEBUG";

    switch (entry.level) {
    case ConsoleLogLevel::Error:
        levelColor = "#ef4444";
        levelTag = "ERROR";
        break;
    case ConsoleLogLevel::Warning:
        levelColor = "#f59e0b";
        levelTag = "WARN ";
        break;
    case ConsoleLogLevel::Info:
        levelColor = "#38bdf8";
        levelTag = "INFO ";
        break;
    case ConsoleLogLevel::Debug:
        levelColor = "#94a3b8";
        levelTag = "DEBUG";
        break;
    default:
        break;
    }

    QString sourceTag = "System";
    if (entry.source == ConsoleLogSource::Metro) {
        sourceTag = "Metro ";
    } else if (entry.source == ConsoleLogSource::Browser) {
        sourceTag = "Browser";
    }

    return QString("<span style='color: #64748b;'>[%1]</span> "
                   "<span style='color: #a855f7; font-weight: 600;'>[%2]</span> "
                   "<span style='color: %3; font-weight: 700;'>[%4]</span> "
                   "<span style='color: %3;'>%5</span>")
        .arg(timeStr)
        .arg(sourceTag)
        .arg(levelColor)
        .arg(levelTag)
        .arg(entry.message.toHtmlEscaped());
}

void DeveloperConsolePanel::refreshDisplay() {
    m_displayArea->clear();
    for (const auto& entry : m_entries) {
        if (matchesFilter(entry)) {
            m_displayArea->append(formatEntryHtml(entry));
        }
    }
}

void DeveloperConsolePanel::onSearchTextChanged(const QString& text) {
    m_searchText = text.trimmed();
    refreshDisplay();
}

void DeveloperConsolePanel::onLevelFilterChanged(int index) {
    m_filterLevel = static_cast<ConsoleLogLevel>(m_levelCombo->itemData(index).toInt());
    refreshDisplay();
}

void DeveloperConsolePanel::onSourceFilterChanged(int index) {
    m_filterSource = static_cast<ConsoleLogSource>(m_sourceCombo->itemData(index).toInt());
    refreshDisplay();
}

void DeveloperConsolePanel::setFilterText(const QString& text) {
    m_searchBox->setText(text);
}

void DeveloperConsolePanel::setFilterLevel(ConsoleLogLevel level) {
    int idx = m_levelCombo->findData(static_cast<int>(level));
    if (idx >= 0) m_levelCombo->setCurrentIndex(idx);
}

void DeveloperConsolePanel::setFilterSource(ConsoleLogSource source) {
    int idx = m_sourceCombo->findData(static_cast<int>(source));
    if (idx >= 0) m_sourceCombo->setCurrentIndex(idx);
}

QString DeveloperConsolePanel::exportAsText() const {
    QStringList lines;
    for (const auto& entry : m_entries) {
        QString timeStr = entry.timestamp.toString("yyyy-MM-dd hh:mm:ss.zzz");
        QString levelStr = "INFO";
        if (entry.level == ConsoleLogLevel::Error) levelStr = "ERROR";
        else if (entry.level == ConsoleLogLevel::Warning) levelStr = "WARN";
        else if (entry.level == ConsoleLogLevel::Debug) levelStr = "DEBUG";

        QString sourceStr = "System";
        if (entry.source == ConsoleLogSource::Metro) sourceStr = "Metro";
        else if (entry.source == ConsoleLogSource::Browser) sourceStr = "Browser";

        lines.append(QString("[%1] [%2] [%3] %4").arg(timeStr).arg(sourceStr).arg(levelStr).arg(entry.message));
    }
    return lines.join("\n");
}

QString DeveloperConsolePanel::exportAsJson() const {
    QJsonArray array;
    for (const auto& entry : m_entries) {
        QJsonObject obj;
        obj["id"] = entry.id;
        obj["timestamp"] = entry.timestamp.toString(Qt::ISODateWithMs);
        obj["level"] = static_cast<int>(entry.level);
        obj["source"] = static_cast<int>(entry.source);
        obj["message"] = entry.message;
        if (!entry.stackTrace.isEmpty()) {
            obj["stackTrace"] = entry.stackTrace;
        }
        array.append(obj);
    }
    QJsonDocument doc(array);
    return doc.toJson(QJsonDocument::Indented);
}

bool DeveloperConsolePanel::exportToFile(const QString& filePath) const {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QString content = filePath.endsWith(".json", Qt::CaseInsensitive) ? exportAsJson() : exportAsText();
    return file.write(content.toUtf8()) != -1;
}

void DeveloperConsolePanel::onCopyClicked() {
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(exportAsText());
}

void DeveloperConsolePanel::onExportClicked() {
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Export Console Diagnostics",
        "ionexa_console.log",
        "Log Files (*.log *.txt);;JSON Files (*.json)"
    );
    if (!fileName.isEmpty()) {
        exportToFile(fileName);
    }
}

void DeveloperConsolePanel::onClearClicked() {
    clear();
}

} // namespace ionexa::ui
