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

    [[nodiscard]] int customViewportWidth() const noexcept { return m_customViewportWidth; }
    void setCustomViewportWidth(int w) { m_customViewportWidth = w; }

    [[nodiscard]] int customViewportHeight() const noexcept { return m_customViewportHeight; }
    void setCustomViewportHeight(int h) { m_customViewportHeight = h; }

    [[nodiscard]] bool touchEmulationEnabled() const noexcept { return m_touchEmulationEnabled; }
    void setTouchEmulationEnabled(bool enabled) { m_touchEmulationEnabled = enabled; }

    [[nodiscard]] double zoomFactor() const noexcept { return m_zoomFactor; }
    void setZoomFactor(double zoom) { m_zoomFactor = zoom; }

    [[nodiscard]] bool experimentalNativeWindowsRuntime() const noexcept { return m_experimentalNativeWindowsRuntime; }
    void setExperimentalNativeWindowsRuntime(bool enable) { m_experimentalNativeWindowsRuntime = enable; }

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
    int m_customViewportWidth{390};
    int m_customViewportHeight{844};
    bool m_touchEmulationEnabled{true};
    double m_zoomFactor{1.0};
    bool m_experimentalNativeWindowsRuntime{false};
    std::vector<std::string> m_recentProjects;
};

} // namespace ionexa::core
