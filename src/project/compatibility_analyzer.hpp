#pragma once

#include <QString>
#include <QStringList>
#include <QVector>
#include <QMap>
#include <QDateTime>

namespace ionexa::project {

enum class CompatibilityTier {
    Supported,
    Partial,
    Unsupported,
    Unverified
};

struct ModuleCompatibilityInfo {
    QString moduleName;
    CompatibilityTier tier{CompatibilityTier::Unverified};
    QString webSupportNotes;
    QString nativeIosNotes;
    QString suggestedAlternative;
};

struct FilePlatformAnalysis {
    int totalSourceFiles{0};
    int iosSpecificFiles{0};
    int webSpecificFiles{0};
    int nativeSpecificFiles{0};
    int androidSpecificFiles{0};
    QStringList iosFileList;
    QStringList webFileList;
};

struct CompatibilityReport {
    QString projectName;
    int totalModulesAnalyzed{0};
    int supportedCount{0};
    int partialCount{0};
    int unsupportedCount{0};
    int unverifiedCount{0};
    double compatibilityScore{100.0};
    QVector<ModuleCompatibilityInfo> moduleResults;
    FilePlatformAnalysis fileAnalysis;
    QStringList recommendations;
    QDateTime scanTimestamp;

    [[nodiscard]] QString tierSummary() const;
    [[nodiscard]] QString exportAsMarkdown() const;
    [[nodiscard]] QString exportAsJson() const;
};

class CompatibilityAnalyzer {
public:
    static CompatibilityReport analyze(const QString& projectPath);
    static ModuleCompatibilityInfo evaluateModule(const QString& moduleName);
    static FilePlatformAnalysis scanSourceFiles(const QString& projectPath);

    static QString tierToString(CompatibilityTier tier);
    static QString tierBadgeColor(CompatibilityTier tier);

private:
    static const QMap<QString, ModuleCompatibilityInfo>& knownModuleDatabase();
};

} // namespace ionexa::project
