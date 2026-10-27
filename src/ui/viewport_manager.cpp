#include "viewport_manager.hpp"
#include <QSettings>
#include <QDateTime>

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

bool ViewportManager::addCustomProfile(const DeviceProfile& profile) {
    if (profile.id.isEmpty() || profile.name.isEmpty()) return false;
    for (const auto& p : m_profiles) {
        if (p.id == profile.id) return false;
    }
    m_profiles.push_back(profile);
    emit profileListChanged();
    return true;
}

bool ViewportManager::removeProfile(const QString& id) {
    for (int i = 0; i < m_profiles.size(); ++i) {
        if (m_profiles[i].id == id) {
            if (id == "iphone-14" || id == "pixel-7") return false;
            m_profiles.removeAt(i);
            if (m_activeIndex >= m_profiles.size()) {
                m_activeIndex = m_profiles.size() - 1;
            }
            emit profileListChanged();
            emit profileChanged(activeProfile());
            emit viewportChanged(currentPixelWidth(), currentPixelHeight(), m_isLandscape, m_zoomFactor);
            return true;
        }
    }
    return false;
}

bool ViewportManager::updateProfile(const DeviceProfile& profile) {
    for (int i = 0; i < m_profiles.size(); ++i) {
        if (m_profiles[i].id == profile.id) {
            m_profiles[i] = profile;
            emit profileListChanged();
            if (i == m_activeIndex) {
                emit profileChanged(activeProfile());
                emit viewportChanged(currentPixelWidth(), currentPixelHeight(), m_isLandscape, m_zoomFactor);
            }
            return true;
        }
    }
    return false;
}

bool ViewportManager::duplicateProfile(const QString& sourceId, const QString& newName) {
    for (const auto& p : m_profiles) {
        if (p.id == sourceId) {
            DeviceProfile dup = p;
            dup.id = QString("%1-copy-%2").arg(sourceId).arg(QDateTime::currentMSecsSinceEpoch());
            dup.name = newName.isEmpty() ? QString("%1 (Copy)").arg(p.name) : newName;
            m_profiles.push_back(dup);
            emit profileListChanged();
            return true;
        }
    }
    return false;
}

void ViewportManager::resetToDefaultProfiles() {
    initStandardProfiles();
    m_activeIndex = 0;
    emit profileListChanged();
    emit profileChanged(activeProfile());
    emit viewportChanged(currentPixelWidth(), currentPixelHeight(), m_isLandscape, m_zoomFactor);
}

void ViewportManager::saveProfiles() {
    QSettings settings("UshaLabs", "IOneXa");
    settings.beginWriteArray("custom_profiles");
    int customIdx = 0;
    for (const auto& p : m_profiles) {
        if (p.id.startsWith("custom-") || p.id.contains("-copy-")) {
            settings.setArrayIndex(customIdx++);
            settings.setValue("id", p.id);
            settings.setValue("name", p.name);
            settings.setValue("width", p.width);
            settings.setValue("height", p.height);
            settings.setValue("scaleFactor", p.scaleFactor);
            settings.setValue("isMobile", p.isMobile);
            settings.setValue("hasDynamicIsland", p.hasDynamicIsland);
            settings.setValue("hasNotch", p.hasNotch);
            settings.setValue("userAgent", p.userAgent);
        }
    }
    settings.endArray();
}

void ViewportManager::loadProfiles() {
    QSettings settings("UshaLabs", "IOneXa");
    int count = settings.beginReadArray("custom_profiles");
    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);
        DeviceProfile p;
        p.id = settings.value("id").toString();
        p.name = settings.value("name").toString();
        p.width = settings.value("width", 390).toInt();
        p.height = settings.value("height", 844).toInt();
        p.scaleFactor = settings.value("scaleFactor", 2.0).toDouble();
        p.isMobile = settings.value("isMobile", true).toBool();
        p.hasDynamicIsland = settings.value("hasDynamicIsland", false).toBool();
        p.hasNotch = settings.value("hasNotch", false).toBool();
        p.userAgent = settings.value("userAgent").toString();
        if (!p.id.isEmpty() && !p.name.isEmpty()) {
            addCustomProfile(p);
        }
    }
    settings.endArray();
}

} // namespace ionexa::ui
