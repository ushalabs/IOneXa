#pragma once

#include <QWidget>
#include <QString>
#include <QVector>
#include <QDateTime>
#include <QTextEdit>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace ionexa::ui {

enum class ConsoleLogLevel {
    All = 0,
    Debug,
    Info,
    Warning,
    Error
};

enum class ConsoleLogSource {
    All = 0,
    Metro,
    Browser,
    System
};

struct ConsoleLogEntry {
    qint64 id{0};
    QDateTime timestamp;
    ConsoleLogLevel level{ConsoleLogLevel::Info};
    ConsoleLogSource source{ConsoleLogSource::System};
    QString message;
    QString stackTrace;
};

class DeveloperConsolePanel : public QWidget {
    Q_OBJECT

public:
    explicit DeveloperConsolePanel(QWidget* parent = nullptr);
    ~DeveloperConsolePanel() override = default;

    void appendLog(ConsoleLogLevel level, ConsoleLogSource source, const QString& message, const QString& stack = "");
    void clear();

    [[nodiscard]] QString exportAsText() const;
    [[nodiscard]] QString exportAsJson() const;
    bool exportToFile(const QString& filePath) const;

    [[nodiscard]] const QVector<ConsoleLogEntry>& allEntries() const { return m_entries; }
    [[nodiscard]] QVector<ConsoleLogEntry> filteredEntries() const;

    [[nodiscard]] int countByLevel(ConsoleLogLevel level) const;
    [[nodiscard]] int totalCount() const { return m_entries.size(); }

    void setFilterText(const QString& text);
    void setFilterLevel(ConsoleLogLevel level);
    void setFilterSource(ConsoleLogSource source);

signals:
    void logAdded(const ConsoleLogEntry& entry);
    void consoleCleared();

private slots:
    void onSearchTextChanged(const QString& text);
    void onLevelFilterChanged(int index);
    void onSourceFilterChanged(int index);
    void onClearClicked();
    void onCopyClicked();
    void onExportClicked();

private:
    void setupUi();
    void refreshDisplay();
    void updateCounters();
    [[nodiscard]] bool matchesFilter(const ConsoleLogEntry& entry) const;
    [[nodiscard]] QString formatEntryHtml(const ConsoleLogEntry& entry) const;

    QVector<ConsoleLogEntry> m_entries;
    qint64 m_nextId{1};
    int m_maxEntries{5000};

    // Filter state
    QString m_searchText;
    ConsoleLogLevel m_filterLevel{ConsoleLogLevel::All};
    ConsoleLogSource m_filterSource{ConsoleLogSource::All};

    // UI elements
    QLineEdit* m_searchBox{nullptr};
    QComboBox* m_levelCombo{nullptr};
    QComboBox* m_sourceCombo{nullptr};
    QLabel* m_badgeErrors{nullptr};
    QLabel* m_badgeWarnings{nullptr};
    QLabel* m_badgeInfo{nullptr};
    QPushButton* m_btnClear{nullptr};
    QPushButton* m_btnCopy{nullptr};
    QPushButton* m_btnExport{nullptr};
    QTextEdit* m_displayArea{nullptr};
};

} // namespace ionexa::ui
