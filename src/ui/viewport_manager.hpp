#pragma once

#include <QString>
#include <QVector>
#include <QObject>
#include <algorithm>

namespace ionexa::ui {

struct DeviceProfile {
    QString id;
    QString name;
    int width{390};
    int height{844};
    double scaleFactor{3.0};
    bool isMobile{true};
    bool hasDynamicIsland{false};
    bool hasNotch{false};
    QString userAgent;
};

class ViewportManager : public QObject {
    Q_OBJECT

public:
    explicit ViewportManager(QObject* parent = nullptr);
    ~ViewportManager() override = default;

    [[nodiscard]] const QVector<DeviceProfile>& profiles() const { return m_profiles; }
    [[nodiscard]] DeviceProfile activeProfile() const;
    [[nodiscard]] int activeProfileIndex() const { return m_activeIndex; }
    [[nodiscard]] bool isLandscape() const { return m_isLandscape; }
    [[nodiscard]] double zoomFactor() const { return m_zoomFactor; }
    [[nodiscard]] bool isTouchEmulationEnabled() const { return m_touchEmulation; }

    [[nodiscard]] int currentPixelWidth() const;
    [[nodiscard]] int currentPixelHeight() const;

    void selectProfile(int index);
    bool selectProfileById(const QString& id);
    void setCustomDimensions(int width, int height);
    void toggleOrientation();
    void setOrientation(bool isLandscape);
    void setZoomFactor(double zoom);
    void setTouchEmulation(bool enabled);

    // Chrome DevTools Protocol emulation helpers
    [[nodiscard]] QString buildDeviceMetricsJson() const;
    [[nodiscard]] QString buildTouchEmulationJson() const;
    [[nodiscard]] QString buildEmitTouchForMouseJson() const;
    [[nodiscard]] QString buildUserAgentJson() const;

signals:
    void viewportChanged(int width, int height, bool isLandscape, double zoom);
    void profileChanged(const DeviceProfile& profile);
    void touchEmulationToggled(bool enabled);

private:
    void initStandardProfiles();

    QVector<DeviceProfile> m_profiles;
    int m_activeIndex{0};
    bool m_isLandscape{false};
    double m_zoomFactor{1.0};
    bool m_touchEmulation{true};
};

} // namespace ionexa::ui
