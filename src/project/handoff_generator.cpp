#include "handoff_generator.hpp"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

namespace ionexa::project {

HandoffPlan HandoffGenerator::generate(const ProjectDetails& project, const CompatibilityReport& report) {
    HandoffPlan plan;
    plan.projectName = QString::fromStdString(project.projectName.empty() ? "Expo App" : project.projectName);
    plan.expoSdkVersion = QString::fromStdString(project.expoSdkVersion.empty() ? "52.0.0" : project.expoSdkVersion);
    plan.generatedTimestamp = QDateTime::currentDateTime();

    plan.expoGoCommand = "npx expo start --tunnel";
    plan.xcodeCommand = "npx expo run:ios";
    plan.easBuildCommand = "eas build --platform ios --profile preview";

    plan.checklist = buildChecklist(report);
    plan.iosSpecificFiles = report.fileAnalysis.iosFileList;
    plan.recommendations = report.recommendations;

    return plan;
}

QVector<HandoffChecklistItem> HandoffGenerator::buildChecklist(const CompatibilityReport& report) {
    QVector<HandoffChecklistItem> items;

    // Standard baseline verification items
    items.push_back({
        "Viewport & Safe Area",
        "Verify Safe Area insets around Dynamic Island / Home Indicator notch on physical iPhone",
        "",
        false
    });

    items.push_back({
        "Touch & Gestures",
        "Verify native multi-touch gestures, pan responders, and scroll velocity bounce",
        "",
        false
    });

    items.push_back({
        "Typography & Fonts",
        "Verify custom font rendering and San Francisco system font scaling under iOS Dynamic Type",
        "",
        false
    });

    // Module-specific verification items
    for (const auto& mod : report.moduleResults) {
        if (mod.tier == CompatibilityTier::Unsupported) {
            items.push_back({
                "Native Hardware & Frameworks",
                QString("Verify full native execution of '%1' on physical device (cannot be verified in web preview)")
                    .arg(mod.moduleName),
                mod.moduleName,
                true
            });
        } else if (mod.tier == CompatibilityTier::Partial) {
            items.push_back({
                "Platform Parity Audit",
                QString("Audit '%1' native iOS behavior against web polyfill fallback (%2)")
                    .arg(mod.moduleName, mod.webSupportNotes.isEmpty() ? mod.nativeIosNotes : mod.webSupportNotes),
                mod.moduleName,
                false
            });
        }
    }

    // Platform-specific file verification
    if (report.fileAnalysis.iosSpecificFiles > 0) {
        items.push_back({
            "Platform Code Verification",
            QString("Execute smoke tests for %1 platform-specific iOS files (*.ios.js/ts)")
                .arg(report.fileAnalysis.iosSpecificFiles),
            "ios-specific-code",
            true
        });
    }

    return items;
}

QString HandoffGenerator::generateMarkdown(const ProjectDetails& project, const CompatibilityReport& report) {
    return generate(project, report).exportAsMarkdown();
}

QString HandoffGenerator::generateJson(const ProjectDetails& project, const CompatibilityReport& report) {
    return generate(project, report).exportAsJson();
}

QString HandoffPlan::exportAsMarkdown() const {
    QString out;
    out += QString("# Native iOS Verification Handoff Guide: %1\n\n").arg(projectName);
    out += QString("- **Generated:** %1\n").arg(generatedTimestamp.toString(Qt::ISODate));
    out += QString("- **Expo SDK:** %1\n").arg(expoSdkVersion);
    out += "- **Truth in Engineering Disclosure:** IOneXa provides an interactive local Expo Web preview on Windows. "
           "Final App Store certification and native iOS runtime verification require physical device testing or macOS Xcode execution.\n\n";

    out += "## 1. Quick Verification on Physical iPhone (Zero-Install Mac Setup)\n\n";
    out += "You can immediately test on a physical iPhone over your local Wi-Fi network using the **Expo Go** client:\n\n";
    out += "1. Install **Expo Go** from the Apple App Store on your iPhone.\n";
    out += "2. In your Windows terminal, run:\n";
    out += QString("   ```bash\n   %1\n   ```\n").arg(expoGoCommand);
    out += "3. Open the iOS Camera app and scan the QR code displayed in the terminal.\n";
    out += "4. The JavaScript bundle will download directly over your LAN/Tunnel onto your iPhone.\n\n";

    out += "## 2. macOS Xcode Simulator / Production Native Build\n\n";
    out += "For testing native modules requiring custom native code (Development Client / EAS Build):\n\n";
    out += "### Option A: Local macOS Machine\n";
    out += "```bash\n";
    out += "git pull\n";
    out += "npx pod-install\n";
    out += QString("%1\n").arg(xcodeCommand);
    out += "```\n\n";

    out += "### Option B: Cloud Build with EAS (No Mac Required)\n";
    out += "```bash\n";
    out += QString("%1\n").arg(easBuildCommand);
    out += "```\n\n";

    out += "## 3. Pre-Flight Native Verification Checklist\n\n";
    out += "Complete the following signoff items before releasing to production:\n\n";

    for (const auto& item : checklist) {
        QString gate = item.isBlocking ? " **[BLOCKING GATE]**" : "";
        out += QString("- [ ] **[%1]** %2%3\n")
            .arg(item.category, item.itemText, gate);
    }

    if (!iosSpecificFiles.isEmpty()) {
        out += "\n## 4. Platform-Specific Files Requiring Native Review\n\n";
        for (const auto& file : iosSpecificFiles) {
            out += QString("- `%1`\n").arg(file);
        }
    }

    if (!recommendations.isEmpty()) {
        out += "\n## 5. Architectural Recommendations\n\n";
        for (const auto& rec : recommendations) {
            out += QString("- %1\n").arg(rec);
        }
    }

    return out;
}

QString HandoffPlan::exportAsJson() const {
    QJsonObject root;
    root["projectName"] = projectName;
    root["expoSdkVersion"] = expoSdkVersion;
    root["generatedTimestamp"] = generatedTimestamp.toString(Qt::ISODate);
    root["expoGoCommand"] = expoGoCommand;
    root["xcodeCommand"] = xcodeCommand;
    root["easBuildCommand"] = easBuildCommand;

    QJsonArray checkArray;
    for (const auto& item : checklist) {
        QJsonObject cObj;
        cObj["category"] = item.category;
        cObj["itemText"] = item.itemText;
        cObj["moduleName"] = item.moduleName;
        cObj["isBlocking"] = item.isBlocking;
        checkArray.append(cObj);
    }
    root["checklist"] = checkArray;

    QJsonArray filesArray;
    for (const auto& f : iosSpecificFiles) {
        filesArray.append(f);
    }
    root["iosSpecificFiles"] = filesArray;

    QJsonArray recsArray;
    for (const auto& r : recommendations) {
        recsArray.append(r);
    }
    root["recommendations"] = recsArray;

    QJsonDocument doc(root);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Indented));
}

} // namespace ionexa::project
