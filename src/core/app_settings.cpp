#include "app_settings.hpp"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <algorithm>

namespace ionexa::core {

AppSettings::AppSettings() = default;

void AppSettings::addRecentProject(const std::string& projectPath) {
    if (projectPath.empty()) return;

    // Remove if already present, then prepend
    auto it = std::remove(m_recentProjects.begin(), m_recentProjects.end(), projectPath);
    m_recentProjects.erase(it, m_recentProjects.end());
    m_recentProjects.insert(m_recentProjects.begin(), projectPath);

    // Limit to 10 most recent
    if (m_recentProjects.size() > 10) {
        m_recentProjects.resize(10);
    }
}

void AppSettings::clearRecentProjects() {
    m_recentProjects.clear();
}

std::string AppSettings::toJson() const {
    QJsonObject root;
    root["defaultDevicePreset"] = QString::fromStdString(m_defaultDevicePreset);
    root["defaultPort"] = m_defaultPort;
    root["startInLandscape"] = m_startInLandscape;
    root["telemetryOptIn"] = m_telemetryOptIn;

    QJsonArray recents;
    for (const auto& p : m_recentProjects) {
        recents.append(QString::fromStdString(p));
    }
    root["recentProjects"] = recents;

    QJsonDocument doc(root);
    return doc.toJson(QJsonDocument::Indented).toStdString();
}

bool AppSettings::fromJson(const std::string& jsonString) {
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(jsonString), &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        return false;
    }

    QJsonObject root = doc.object();
    if (root.contains("defaultDevicePreset") && root["defaultDevicePreset"].isString()) {
        m_defaultDevicePreset = root["defaultDevicePreset"].toString().toStdString();
    }
    if (root.contains("defaultPort") && root["defaultPort"].isDouble()) {
        m_defaultPort = root["defaultPort"].toInt();
    }
    if (root.contains("startInLandscape") && root["startInLandscape"].isBool()) {
        m_startInLandscape = root["startInLandscape"].toBool();
    }
    if (root.contains("telemetryOptIn") && root["telemetryOptIn"].isBool()) {
        m_telemetryOptIn = root["telemetryOptIn"].toBool();
    }
    if (root.contains("recentProjects") && root["recentProjects"].isArray()) {
        m_recentProjects.clear();
        for (const auto& val : root["recentProjects"].toArray()) {
            if (val.isString()) {
                m_recentProjects.push_back(val.toString().toStdString());
            }
        }
    }

    return true;
}

bool AppSettings::loadFromFile(const std::string& filePath) {
    QFile file(QString::fromStdString(filePath));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    QByteArray data = file.readAll();
    return fromJson(data.toStdString());
}

bool AppSettings::saveToFile(const std::string& filePath) const {
    QFile file(QString::fromStdString(filePath));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    std::string json = toJson();
    return file.write(json.data(), json.size()) == static_cast<qint64>(json.size());
}

} // namespace ionexa::core
