#include "viewport_manager.hpp"

namespace ionexa::ui {

ViewportManager::ViewportManager(QObject* parent)
    : QObject(parent) {
    initStandardProfiles();
}

void ViewportManager::initStandardProfiles() {
    m_profiles.clear();

    // 1. iPhone 14/15
    m_profiles.push_back({
        "iphone-14",
        "iPhone 14 / 15",
        390,
        844,
        3.0,
        true,
        true,  // Dynamic Island
        false,
        "Mozilla/5.0 (iPhone; CPU iPhone OS 17_4 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.4 Mobile/15E148 Safari/604.1"
    });

    // 2. iPhone 15 Pro Max
    m_profiles.push_back({
        "iphone-15-pro-max",
        "iPhone 15 Pro Max",
        430,
        932,
        3.0,
        true,
        true,  // Dynamic Island
        false,
        "Mozilla/5.0 (iPhone; CPU iPhone OS 17_4 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.4 Mobile/15E148 Safari/604.1"
    });

    // 3. iPhone SE (3rd Gen)
    m_profiles.push_back({
        "iphone-se",
        "iPhone SE (3rd Gen)",
        375,
        667,
        2.0,
        true,
        false,
        false,
        "Mozilla/5.0 (iPhone; CPU iPhone OS 17_0 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.0 Mobile/15E148 Safari/604.1"
    });

    // 4. Google Pixel 7
    m_profiles.push_back({
        "pixel-7",
        "Google Pixel 7",
        412,
        915,
        2.625,
        true,
        false,
        false,
        "Mozilla/5.0 (Linux; Android 14; Pixel 7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/122.0.0.0 Mobile Safari/537.36"
    });

    // 5. Samsung Galaxy S23
    m_profiles.push_back({
        "galaxy-s23",
        "Samsung Galaxy S23",
        360,
        780,
        3.0,
        true,
        false,
        false,
        "Mozilla/5.0 (Linux; Android 14; SM-S911B) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/122.0.0.0 Mobile Safari/537.36"
    });

    // 6. iPad Mini (6th Gen)
    m_profiles.push_back({
        "ipad-mini",
        "iPad Mini (6th Gen)",
        744,
        1133,
        2.0,
        false,
        false,
        false,
        "Mozilla/5.0 (iPad; CPU OS 17_4 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.4 Mobile/15E148 Safari/604.1"
    });

    // 7. Custom Viewport
    m_profiles.push_back({
        "custom",
        "Custom Viewport",
        390,
        844,
        1.0,
        true,
        false,
        false,
        ""
    });
}

DeviceProfile ViewportManager::activeProfile() const {
    if (m_activeIndex >= 0 && m_activeIndex < m_profiles.size()) {
        return m_profiles[m_activeIndex];
    }
    return m_profiles.isEmpty() ? DeviceProfile{} : m_profiles[0];
}

int ViewportManager::currentPixelWidth() const {
    const auto prof = activeProfile();
    return m_isLandscape ? prof.height : prof.width;
}

int ViewportManager::currentPixelHeight() const {
    const auto prof = activeProfile();
    return m_isLandscape ? prof.width : prof.height;
}

void ViewportManager::selectProfile(int index) {
    if (index >= 0 && index < m_profiles.size() && index != m_activeIndex) {
        m_activeIndex = index;
        emit profileChanged(activeProfile());
        emit viewportChanged(currentPixelWidth(), currentPixelHeight(), m_isLandscape, m_zoomFactor);
    }
}

bool ViewportManager::selectProfileById(const QString& id) {
    for (int i = 0; i < m_profiles.size(); ++i) {
        if (m_profiles[i].id == id) {
            selectProfile(i);
            return true;
        }
    }
    return false;
}

void ViewportManager::setCustomDimensions(int width, int height) {
    // Clamp to valid screen bounds [320, 3840] x [480, 2160]
    int clampedW = std::clamp(width, 320, 3840);
    int clampedH = std::clamp(height, 480, 2160);

    for (auto& prof : m_profiles) {
        if (prof.id == "custom") {
            prof.width = clampedW;
            prof.height = clampedH;
            break;
        }
    }

    selectProfileById("custom");
    emit viewportChanged(currentPixelWidth(), currentPixelHeight(), m_isLandscape, m_zoomFactor);
}

void ViewportManager::toggleOrientation() {
    setOrientation(!m_isLandscape);
}

void ViewportManager::setOrientation(bool isLandscape) {
    if (m_isLandscape != isLandscape) {
        m_isLandscape = isLandscape;
        emit viewportChanged(currentPixelWidth(), currentPixelHeight(), m_isLandscape, m_zoomFactor);
    }
}

void ViewportManager::setZoomFactor(double zoom) {
    double clampedZoom = std::clamp(zoom, 0.5, 3.0);
    if (std::abs(m_zoomFactor - clampedZoom) > 0.001) {
        m_zoomFactor = clampedZoom;
        emit viewportChanged(currentPixelWidth(), currentPixelHeight(), m_isLandscape, m_zoomFactor);
    }
}

void ViewportManager::setTouchEmulation(bool enabled) {
    if (m_touchEmulation != enabled) {
        m_touchEmulation = enabled;
        emit touchEmulationToggled(enabled);
    }
}

QString ViewportManager::buildDeviceMetricsJson() const {
    const auto prof = activeProfile();
    int w = currentPixelWidth();
    int h = currentPixelHeight();
    return QString(R"({"width":%1,"height":%2,"deviceScaleFactor":%3,"mobile":%4})")
        .arg(w)
        .arg(h)
        .arg(prof.scaleFactor)
        .arg(prof.isMobile ? "true" : "false");
}

QString ViewportManager::buildTouchEmulationJson() const {
    return QString(R"({"enabled":%1,"maxTouchPoints":5})")
        .arg(m_touchEmulation ? "true" : "false");
}

QString ViewportManager::buildEmitTouchForMouseJson() const {
    return QString(R"({"enabled":%1,"configuration":"mobile"})")
        .arg(m_touchEmulation ? "true" : "false");
}

QString ViewportManager::buildUserAgentJson() const {
    const auto prof = activeProfile();
    if (prof.userAgent.isEmpty()) {
        return "{}";
    }
    return QString(R"({"userAgent":"%1"})").arg(prof.userAgent);
}

} // namespace ionexa::ui
