#pragma once

#include <QString>
#include <QColor>
#include <QFont>

namespace ionexa::ui {

struct ThemeColors {
    static constexpr const char* MainBackground = "#16181D";
    static constexpr const char* SecondarySurface = "#1E222A";
    static constexpr const char* RaisedSurface = "#262B35";
    static constexpr const char* CardBackground = "#21252E";
    static constexpr const char* Border = "#343B48";
    static constexpr const char* BorderSubtle = "#2A303C";
    static constexpr const char* BorderFocus = "#4E9CFF";
    static constexpr const char* PrimaryAccent = "#4E9CFF";
    static constexpr const char* PrimaryAccentHover = "#68ABFF";
    static constexpr const char* PrimaryAccentPressed = "#3584E4";
    static constexpr const char* Success = "#43C990";
    static constexpr const char* Warning = "#F5B84D";
    static constexpr const char* Error = "#F06B72";
    static constexpr const char* PrimaryText = "#F2F4F8";
    static constexpr const char* SecondaryText = "#A4ADBD";
    static constexpr const char* MutedText = "#6B7688";
    static constexpr const char* DeviceChrome = "#0E1015";
    static constexpr const char* DeviceBezel = "#2A2E38";
};

class Theme {
public:
    // Color accessors
    [[nodiscard]] static QColor color(const char* hex);
    [[nodiscard]] static QColor mainBg();
    [[nodiscard]] static QColor secondaryBg();
    [[nodiscard]] static QColor accent();
    [[nodiscard]] static QColor text();
    [[nodiscard]] static QColor muted();

    // Typography
    [[nodiscard]] static QFont defaultFont(int pointSize = 9, int weight = QFont::Normal);
    [[nodiscard]] static QFont titleFont(int pointSize = 13, int weight = QFont::DemiBold);
    [[nodiscard]] static QFont monoFont(int pointSize = 9);

    // Stylesheets
    [[nodiscard]] static QString workbenchStyle();
    [[nodiscard]] static QString emulatorStyle();
    [[nodiscard]] static QString commandBarStyle();
    [[nodiscard]] static QString navRailStyle();
    [[nodiscard]] static QString cardStyle();
    [[nodiscard]] static QString buttonStyle(bool isPrimary = false, bool isDestructive = false);
    [[nodiscard]] static QString iconButtonStyle();
    [[nodiscard]] static QString badgeStyle(const QString& bgHex, const QString& textHex);
    [[nodiscard]] static QString tabBarStyle();
    [[nodiscard]] static QString splitterStyle();
    [[nodiscard]] static QString scrollBarStyle();
    [[nodiscard]] static QString lineEditStyle();
    [[nodiscard]] static QString comboBoxStyle();
};

} // namespace ionexa::ui
