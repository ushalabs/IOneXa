#include "compatibility_analyzer.hpp"

#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <algorithm>

namespace ionexa::project {

const QMap<QString, ModuleCompatibilityInfo>& CompatibilityAnalyzer::knownModuleDatabase() {
    static const QMap<QString, ModuleCompatibilityInfo> db = {
        // Supported
        {"expo", {"expo", CompatibilityTier::Supported, "Core Expo runtime with web platform support", "Standard native iOS runner", ""}},
        {"@expo/metro-runtime", {"@expo/metro-runtime", CompatibilityTier::Supported, "Metro web fast refresh and runtime client", "Not required on iOS", ""}},
        {"react", {"react", CompatibilityTier::Supported, "Core React UI library", "Core React UI library", ""}},
        {"react-dom", {"react-dom", CompatibilityTier::Supported, "DOM renderer for React web target", "Not used on iOS", ""}},
        {"react-native", {"react-native", CompatibilityTier::Supported, "Cross-platform mobile UI primitives", "Native iOS Obj-C / Swift runtime", ""}},
        {"expo-router", {"expo-router", CompatibilityTier::Supported, "Fully supported URL and file-based routing on web", "Native file-based navigation", ""}},
        {"expo-constants", {"expo-constants", CompatibilityTier::Supported, "Browser manifest and execution constants", "Native app plist constants", ""}},
        {"expo-font", {"expo-font", CompatibilityTier::Supported, "CSS @font-face web font loader", "CoreText font loader", ""}},
        {"expo-image", {"expo-image", CompatibilityTier::Supported, "HTML5 <img> and responsive picture loader", "SDWebImage native iOS caching", ""}},
        {"expo-linking", {"expo-linking", CompatibilityTier::Supported, "HTML5 History API & deep links", "Universal Links & custom schemes", ""}},
        {"expo-status-bar", {"expo-status-bar", CompatibilityTier::Supported, "No-op on web without crashing", "UIStatusBarStyle on iOS", ""}},
        {"expo-asset", {"expo-asset", CompatibilityTier::Supported, "Webpack/Metro web static asset resolution", "Bundled iOS asset catalog loader", ""}},
        {"expo-localization", {"expo-localization", CompatibilityTier::Supported, "navigator.languages web locale detection", "NSLocale native locale", ""}},
        {"expo-vector-icons", {"expo-vector-icons", CompatibilityTier::Supported, "Web font icons via CSS", "Bundled TTF icon fonts", ""}},
        {"react-native-web", {"react-native-web", CompatibilityTier::Supported, "Core Web target mapping React Native primitives to DOM", "Not used on iOS", ""}},
        {"react-native-svg", {"react-native-svg", CompatibilityTier::Supported, "Standard SVG elements in web DOM", "CoreGraphics native SVG rendering", ""}},
        {"@react-navigation/native", {"@react-navigation/native", CompatibilityTier::Supported, "Web history & navigation integration", "Native stack controller", ""}},
        {"@react-navigation/stack", {"@react-navigation/stack", CompatibilityTier::Supported, "CSS transition stack navigation", "UINavigationController", ""}},
        {"@react-navigation/bottom-tabs", {"@react-navigation/bottom-tabs", CompatibilityTier::Supported, "HTML5 bottom tab bar layout", "UITabBarController", ""}},
        {"react-native-safe-area-context", {"react-native-safe-area-context", CompatibilityTier::Supported, "CSS env(safe-area-inset-*) polyfill", "UIEdgeInsets safe area on iOS", ""}},
        {"react-native-screens", {"react-native-screens", CompatibilityTier::Supported, "Web DOM visibility optimizations", "Native UIViewController management", ""}},

        // Partial
        {"@react-native-async-storage/async-storage", {"@react-native-async-storage/async-storage", CompatibilityTier::Partial, "Backed by window.localStorage in web", "Native SQLite / file store on iOS", "Consider storage quotas on web"}},
        {"expo-secure-store", {"expo-secure-store", CompatibilityTier::Partial, "Emulated in web storage (NOT secure against XSS)", "Hardware Keychain Services on iOS", "Do not store highly sensitive credentials in web preview"}},
        {"expo-camera", {"expo-camera", CompatibilityTier::Partial, "WebRTC getUserMedia video capture", "AVCaptureSession with full lens controls on iOS", "Camera permissions required in browser"}},
        {"expo-location", {"expo-location", CompatibilityTier::Partial, "navigator.geolocation browser API", "CoreLocation with background tracking on iOS", "Background location not supported on web"}},
        {"expo-notifications", {"expo-notifications", CompatibilityTier::Partial, "Limited ServiceWorker web push", "Apple Push Notification service (APNs)", "Remote APNs push requires real iOS device"}},
        {"expo-haptics", {"expo-haptics", CompatibilityTier::Partial, "navigator.vibrate in supported browsers", "Taptic Engine UIImpactFeedbackGenerator on iOS", ""}},
        {"react-native-reanimated", {"react-native-reanimated", CompatibilityTier::Partial, "CSS animations & requestAnimationFrame fallback", "Native CADisplayLink and Worklets on iOS", ""}},
        {"expo-sharing", {"expo-sharing", CompatibilityTier::Partial, "navigator.share Web Share API", "UIActivityViewController native sheet on iOS", ""}},
        {"expo-clipboard", {"expo-clipboard", CompatibilityTier::Partial, "navigator.clipboard web API", "UIPasteboard on iOS", ""}},

        // Unsupported (Native iOS-only)
        {"react-native-iap", {"react-native-iap", CompatibilityTier::Unsupported, "In-App Purchases not supported on web", "Requires Apple StoreKit framework", "Stub IAP calls in web preview or use Platform.select"}},
        {"@react-native-community/push-notification-ios", {"@react-native-community/push-notification-ios", CompatibilityTier::Unsupported, "Apple Push Notification framework unsupported on web", "Requires iOS APNs & UserNotifications", "Use expo-notifications or mock on web"}},
        {"react-native-keychain", {"react-native-keychain", CompatibilityTier::Unsupported, "Keychain Services not available in browser", "Requires Apple SecKey / Keychain", "Use expo-secure-store with web fallback"}},
        {"react-native-health", {"react-native-health", CompatibilityTier::Unsupported, "HealthKit unsupported outside iOS/watchOS", "Requires Apple HealthKit framework", "Guard HealthKit calls with Platform.OS === 'ios'"}},
        {"react-native-apple-authentication", {"react-native-apple-authentication", CompatibilityTier::Unsupported, "Native Apple Sign-In dialog unsupported on web", "Requires ASAuthorizationAppleIDProvider", "Use OAuth web flow for Sign in with Apple"}},
        {"react-native-background-fetch", {"react-native-background-fetch", CompatibilityTier::Unsupported, "Background fetch unsupported in tabbed web runtime", "Requires iOS BGTaskScheduler", "Guard with Platform.OS !== 'web'"}},
        {"react-native-ble-plx", {"react-native-ble-plx", CompatibilityTier::Unsupported, "Bluetooth LE library unsupported on web target", "Requires Apple CoreBluetooth", "Use Web Bluetooth API or guard native calls"}},
        {"react-native-idfa", {"react-native-idfa", CompatibilityTier::Unsupported, "App Tracking Transparency unavailable on web", "Requires Apple AdSupport / ATTrackingManager", "Remove or guard on web target"}}
    };
    return db;
}

ModuleCompatibilityInfo CompatibilityAnalyzer::evaluateModule(const QString& moduleName) {
    const auto& db = knownModuleDatabase();
    if (db.contains(moduleName)) {
        return db[moduleName];
    }

    ModuleCompatibilityInfo info;
    info.moduleName = moduleName;
    info.tier = CompatibilityTier::Unverified;
    info.webSupportNotes = "Unverified third-party dependency; verify web bundler compatibility.";
    info.nativeIosNotes = "Native iOS support determined by package podspec.";
    return info;
}

FilePlatformAnalysis CompatibilityAnalyzer::scanSourceFiles(const QString& projectPath) {
    FilePlatformAnalysis result;
    QDir dir(projectPath);
    if (!dir.exists()) return result;

    QDirIterator it(projectPath,
                    {"*.js", "*.jsx", "*.ts", "*.tsx"},
                    QDir::Files,
                    QDirIterator::Subdirectories);

    while (it.hasNext()) {
        QString filePath = it.next();
        QString relPath = dir.relativeFilePath(filePath);

        // Skip ignored directories
        if (relPath.startsWith("node_modules") ||
            relPath.startsWith(".git") ||
            relPath.startsWith(".expo") ||
            relPath.startsWith("build") ||
            relPath.startsWith("dist")) {
            continue;
        }

        result.totalSourceFiles++;

        if (filePath.contains(".ios.")) {
            result.iosSpecificFiles++;
            result.iosFileList.append(relPath);
        } else if (filePath.contains(".web.")) {
            result.webSpecificFiles++;
            result.webFileList.append(relPath);
        } else if (filePath.contains(".native.")) {
            result.nativeSpecificFiles++;
        } else if (filePath.contains(".android.")) {
            result.androidSpecificFiles++;
        }
    }

    return result;
}

CompatibilityReport CompatibilityAnalyzer::analyze(const QString& projectPath) {
    CompatibilityReport report;
    report.scanTimestamp = QDateTime::currentDateTime();

    QFile pkgFile(projectPath + "/package.json");
    if (!pkgFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        report.projectName = "Unknown";
        return report;
    }

    QJsonDocument doc = QJsonDocument::fromJson(pkgFile.readAll());
    if (!doc.isObject()) return report;

    QJsonObject pkgObj = doc.object();
    report.projectName = pkgObj.value("name").toString("unnamed-project");

    QStringList allDependencies;
    if (pkgObj.contains("dependencies") && pkgObj["dependencies"].isObject()) {
        allDependencies.append(pkgObj["dependencies"].toObject().keys());
    }
    if (pkgObj.contains("devDependencies") && pkgObj["devDependencies"].isObject()) {
        allDependencies.append(pkgObj["devDependencies"].toObject().keys());
    }
    allDependencies.removeDuplicates();

    for (const auto& dep : allDependencies) {
        // Skip common build tools and typings
        if (dep.startsWith("@types/") || dep.startsWith("@babel/") || dep == "typescript" || dep == "metro") continue;

        auto info = evaluateModule(dep);
        report.moduleResults.push_back(info);
        report.totalModulesAnalyzed++;

        switch (info.tier) {
        case CompatibilityTier::Supported:
            report.supportedCount++;
            break;
        case CompatibilityTier::Partial:
            report.partialCount++;
            break;
        case CompatibilityTier::Unsupported:
            report.unsupportedCount++;
            break;
        case CompatibilityTier::Unverified:
            report.unverifiedCount++;
            break;
        }
    }

    // Compute Score
    if (report.totalModulesAnalyzed > 0) {
        double weighted = (report.supportedCount * 1.0) +
                          (report.partialCount * 0.75) +
                          (report.unverifiedCount * 0.5) +
                          (report.unsupportedCount * 0.0);
        report.compatibilityScore = (weighted / report.totalModulesAnalyzed) * 100.0;
    } else {
        report.compatibilityScore = 100.0;
    }

    // Source files scan
    report.fileAnalysis = scanSourceFiles(projectPath);

    // Formulate actionable recommendations
    if (report.unsupportedCount > 0) {
        for (const auto& mod : report.moduleResults) {
            if (mod.tier == CompatibilityTier::Unsupported) {
                QString rec = QString("'%1' requires native Apple iOS frameworks. Guard usages with Platform.select or provide a .web.js fallback.").arg(mod.moduleName);
                if (!mod.suggestedAlternative.isEmpty()) {
                    rec += QString(" Suggestion: %1").arg(mod.suggestedAlternative);
                }
                report.recommendations.append(rec);
            }
        }
    }

    if (report.fileAnalysis.iosSpecificFiles > 0 && report.fileAnalysis.webSpecificFiles == 0) {
        report.recommendations.append(QString("Project has %1 .ios.* file(s) but no .web.* equivalents. Ensure web fallback implementations are provided.")
                                      .arg(report.fileAnalysis.iosSpecificFiles));
    }

    if (report.unsupportedCount == 0 && report.compatibilityScore >= 90.0) {
        report.recommendations.append("Project is highly compatible with the IOneXa Expo web workbench runtime.");
    }

    return report;
}

QString CompatibilityAnalyzer::tierToString(CompatibilityTier tier) {
    switch (tier) {
    case CompatibilityTier::Supported: return "Supported";
    case CompatibilityTier::Partial: return "Partial";
    case CompatibilityTier::Unsupported: return "Unsupported";
    case CompatibilityTier::Unverified: return "Unverified";
    }
    return "Unknown";
}

QString CompatibilityAnalyzer::tierBadgeColor(CompatibilityTier tier) {
    switch (tier) {
    case CompatibilityTier::Supported: return "#4ade80"; // green
    case CompatibilityTier::Partial: return "#fbbf24";   // amber
    case CompatibilityTier::Unsupported: return "#f87171"; // red
    case CompatibilityTier::Unverified: return "#94a3b8"; // gray
    }
    return "#94a3b8";
}

QString CompatibilityReport::tierSummary() const {
    return QString("%1 Supported, %2 Partial, %3 Unsupported, %4 Unverified")
        .arg(supportedCount).arg(partialCount).arg(unsupportedCount).arg(unverifiedCount);
}

QString CompatibilityReport::exportAsMarkdown() const {
    QString md = "# Expo & iOS Compatibility Report\n\n";
    md += QString("- **Project:** %1\n").arg(projectName);
    md += QString("- **Compatibility Score:** %1%\n").arg(compatibilityScore, 0, 'f', 1);
    md += QString("- **Scanned Timestamp:** %1\n").arg(scanTimestamp.toString(Qt::ISODate));
    md += QString("- **Modules Analyzed:** %1 (%2)\n\n").arg(totalModulesAnalyzed).arg(tierSummary());

    md += "## Platform Source Files\n\n";
    md += QString("- Total Scanned Files: %1\n").arg(fileAnalysis.totalSourceFiles);
    md += QString("- iOS-Specific Files (*.ios.*): %1\n").arg(fileAnalysis.iosSpecificFiles);
    md += QString("- Web-Specific Files (*.web.*): %1\n").arg(fileAnalysis.webSpecificFiles);
    md += QString("- Native-Specific Files (*.native.*): %1\n\n").arg(fileAnalysis.nativeSpecificFiles);

    md += "## Module Breakdown\n\n";
    md += "| Module | Compatibility Tier | Web Support Notes | Native iOS Notes |\n";
    md += "|---|---|---|---|\n";
    for (const auto& mod : moduleResults) {
        md += QString("| `%1` | **%2** | %3 | %4 |\n")
            .arg(mod.moduleName)
            .arg(CompatibilityAnalyzer::tierToString(mod.tier))
            .arg(mod.webSupportNotes)
            .arg(mod.nativeIosNotes);
    }

    if (!recommendations.isEmpty()) {
        md += "\n## Recommendations\n\n";
        for (const auto& rec : recommendations) {
            md += QString("- %1\n").arg(rec);
        }
    }

    md += "\n---\n*Generated by IOneXa Workbench — Transparent Mobile Engineering*\n";
    return md;
}

QString CompatibilityReport::exportAsJson() const {
    QJsonObject root;
    root["projectName"] = projectName;
    root["compatibilityScore"] = compatibilityScore;
    root["scanTimestamp"] = scanTimestamp.toString(Qt::ISODateWithMs);
    root["totalModulesAnalyzed"] = totalModulesAnalyzed;
    root["supportedCount"] = supportedCount;
    root["partialCount"] = partialCount;
    root["unsupportedCount"] = unsupportedCount;
    root["unverifiedCount"] = unverifiedCount;

    // File analysis
    QJsonObject filesObj;
    filesObj["totalSourceFiles"] = fileAnalysis.totalSourceFiles;
    filesObj["iosSpecificFiles"] = fileAnalysis.iosSpecificFiles;
    filesObj["webSpecificFiles"] = fileAnalysis.webSpecificFiles;
    filesObj["nativeSpecificFiles"] = fileAnalysis.nativeSpecificFiles;
    filesObj["androidSpecificFiles"] = fileAnalysis.androidSpecificFiles;
    root["files"] = filesObj;

    // Modules
    QJsonArray modArr;
    for (const auto& mod : moduleResults) {
        QJsonObject m;
        m["moduleName"] = mod.moduleName;
        m["tier"] = CompatibilityAnalyzer::tierToString(mod.tier);
        m["webSupportNotes"] = mod.webSupportNotes;
        m["nativeIosNotes"] = mod.nativeIosNotes;
        if (!mod.suggestedAlternative.isEmpty()) {
            m["suggestedAlternative"] = mod.suggestedAlternative;
        }
        modArr.append(m);
    }
    root["modules"] = modArr;

    // Recommendations
    QJsonArray recArr;
    for (const auto& rec : recommendations) {
        recArr.append(rec);
    }
    root["recommendations"] = recArr;

    QJsonDocument doc(root);
    return doc.toJson(QJsonDocument::Indented);
}

} // namespace ionexa::project
