#include "project_scanner.hpp"

#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>

namespace ionexa::project {

bool ProjectScanner::checkNodePrerequisites(std::string& detectedVersion) {
    QProcess process;
    process.start("node", QStringList() << "--version");
    if (!process.waitForFinished(3000)) {
        detectedVersion = "";
        return false;
    }
    if (process.exitCode() == 0) {
        detectedVersion = process.readAllStandardOutput().trimmed().toStdString();
        return true;
    }
    return false;
}

ProjectDetails ProjectScanner::scan(const std::string& directoryPath) {
    ProjectDetails details;

    QDir dir(QString::fromStdString(directoryPath));
    if (!dir.exists()) {
        details.isValid = false;
        details.errorMessage = "Directory does not exist";
        return details;
    }

    details.canonicalPath = QDir::toNativeSeparators(dir.canonicalPath()).toStdString();

    // Check Node environment
    details.nodeInstalled = checkNodePrerequisites(details.nodeVersion);

    // Look for package.json
    QString packageJsonPath = dir.filePath("package.json");
    QFileInfo pkgInfo(packageJsonPath);
    if (!pkgInfo.exists() || !pkgInfo.isFile()) {
        details.isValid = false;
        details.errorMessage = "package.json not found in directory";
        return details;
    }

    // Read package.json in strictly read-only mode
    QFile pkgFile(packageJsonPath);
    if (!pkgFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        details.isValid = false;
        details.errorMessage = "Could not open package.json for reading";
        return details;
    }

    QByteArray rawData = pkgFile.readAll();
    pkgFile.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(rawData, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        details.isValid = false;
        details.errorMessage = std::string("Malformed package.json: ") + parseError.errorString().toStdString();
        return details;
    }

    QJsonObject root = doc.object();
    details.projectName = root.value("name").toString().toStdString();
    details.projectVersion = root.value("version").toString().toStdString();
    details.entryPoint = root.value("main").toString().toStdString();

    QJsonObject deps = root.value("dependencies").toObject();
    QJsonObject devDeps = root.value("devDependencies").toObject();

    auto getDepVersion = [&](const QString& key) -> QString {
        if (deps.contains(key)) return deps.value(key).toString();
        if (devDeps.contains(key)) return devDeps.value(key).toString();
        return "";
    };

    // Check Expo dependency
    QString expoVer = getDepVersion("expo");
    if (!expoVer.isEmpty()) {
        details.isExpoProject = true;
        details.expoSdkVersion = expoVer.toStdString();
    } else {
        details.isExpoProject = false;
    }

    QString rnVer = getDepVersion("react-native");
    if (!rnVer.isEmpty()) {
        details.reactNativeVersion = rnVer.toStdString();
    }

    // Check Expo Router
    if (!getDepVersion("expo-router").isEmpty() || dir.exists("app")) {
        details.isExpoRouter = true;
    }

    // Check Web Dependencies
    std::vector<std::string> requiredWeb = {"react-native-web", "react-dom", "@expo/metro-runtime"};
    for (const auto& req : requiredWeb) {
        if (getDepVersion(QString::fromStdString(req)).isEmpty()) {
            details.missingWebDependencies.push_back(req);
        }
    }

    if (details.missingWebDependencies.empty()) {
        details.hasWebSupport = true;
    } else {
        details.hasWebSupport = false;
        std::string remed = "npx expo install";
        for (const auto& missing : details.missingWebDependencies) {
            remed += " " + missing;
        }
        details.remediationCommand = remed;
    }

    // Detect package manager by inspecting lockfiles
    if (dir.exists("pnpm-lock.yaml") || QFileInfo::exists(dir.filePath("../pnpm-lock.yaml"))) {
        details.packageManager = "pnpm";
    } else if (dir.exists("yarn.lock") || QFileInfo::exists(dir.filePath("../yarn.lock"))) {
        details.packageManager = "yarn";
    } else if (dir.exists("bun.lockb") || dir.exists("bun.lock")) {
        details.packageManager = "bun";
    } else if (dir.exists("package-lock.json") || QFileInfo::exists(dir.filePath("../package-lock.json"))) {
        details.packageManager = "npm";
    } else {
        details.packageManager = "npm";
    }

    details.isValid = true;
    return details;
}

} // namespace ionexa::project
