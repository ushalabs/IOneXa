#pragma once

#include <string>
#include <vector>

namespace ionexa::core {

class AppSettings {
public:
    AppSettings();

    [[nodiscard]] const std::string& defaultDevicePreset() const noexcept { return m_defaultDevicePreset; }
    void setDefaultDevicePreset(std::string preset) { m_defaultDevicePreset = std::move(preset); }

    [[nodiscard]] int defaultPort() const noexcept { return m_defaultPort; }
    void setDefaultPort(int port) { m_defaultPort = port; }

    [[nodiscard]] bool startInLandscape() const noexcept { return m_startInLandscape; }
    void setStartInLandscape(bool landscape) { m_startInLandscape = landscape; }

    [[nodiscard]] bool telemetryOptIn() const noexcept { return m_telemetryOptIn; }
    void setTelemetryOptIn(bool optIn) { m_telemetryOptIn = optIn; }

    [[nodiscard]] const std::vector<std::string>& recentProjects() const noexcept { return m_recentProjects; }
    void addRecentProject(const std::string& projectPath);
    void clearRecentProjects();

    bool loadFromFile(const std::string& filePath);
    bool saveToFile(const std::string& filePath) const;

    [[nodiscard]] std::string toJson() const;
    bool fromJson(const std::string& jsonString);

private:
    std::string m_defaultDevicePreset{"iPhone 14/15 (390 x 844)"};
    int m_defaultPort{8081};
    bool m_startInLandscape{false};
    bool m_telemetryOptIn{false};
    std::vector<std::string> m_recentProjects;
};

} // namespace ionexa::core
