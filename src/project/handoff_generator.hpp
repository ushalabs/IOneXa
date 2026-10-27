#pragma once

#include <QString>
#include <QStringList>
#include <QVector>
#include <QJsonObject>
#include <QDateTime>

#include "project/project_scanner.hpp"
#include "project/compatibility_analyzer.hpp"

namespace ionexa::project {

struct HandoffChecklistItem {
    QString category;      // e.g., "Native Device APIs", "Push Notifications", "In-App Purchases"
    QString itemText;      // e.g., "Verify camera barcode scanning on physical iPhone"
    QString moduleName;    // e.g., "expo-camera"
    bool isBlocking;       // Whether this is a critical gate before App Store submission
};

struct HandoffPlan {
    QString projectName;
    QString expoSdkVersion;
    QDateTime generatedTimestamp;
    QString expoGoCommand;
    QString xcodeCommand;
    QString easBuildCommand;
    QVector<HandoffChecklistItem> checklist;
    QStringList iosSpecificFiles;
    QStringList recommendations;

    [[nodiscard]] QString exportAsMarkdown() const;
    [[nodiscard]] QString exportAsJson() const;
};

class HandoffGenerator {
public:
    static HandoffPlan generate(const ProjectDetails& project, const CompatibilityReport& report);
    static QString generateMarkdown(const ProjectDetails& project, const CompatibilityReport& report);
    static QString generateJson(const ProjectDetails& project, const CompatibilityReport& report);

private:
    static QVector<HandoffChecklistItem> buildChecklist(const CompatibilityReport& report);
};

} // namespace ionexa::project
