#include "theme.hpp"

#include <QFontDatabase>

namespace ionexa::ui {

QColor Theme::color(const char* hex) {
    return QColor(hex);
}

QColor Theme::mainBg() {
    return QColor(ThemeColors::MainBackground);
}

QColor Theme::secondaryBg() {
    return QColor(ThemeColors::SecondarySurface);
}

QColor Theme::accent() {
    return QColor(ThemeColors::PrimaryAccent);
}

QColor Theme::text() {
    return QColor(ThemeColors::PrimaryText);
}

QColor Theme::muted() {
    return QColor(ThemeColors::MutedText);
}

QFont Theme::defaultFont(int pointSize, int weight) {
    QFont font("Segoe UI Variable", pointSize, weight);
    if (!font.exactMatch()) {
        font.setFamily("Segoe UI");
    }
    font.setStyleHint(QFont::SansSerif);
    return font;
}

QFont Theme::titleFont(int pointSize, int weight) {
    return defaultFont(pointSize, weight);
}

QFont Theme::monoFont(int pointSize) {
    QFont font("Cascadia Code", pointSize);
    if (!font.exactMatch()) {
        font.setFamily("Consolas");
    }
    font.setStyleHint(QFont::Monospace);
    return font;
}

QString Theme::workbenchStyle() {
    return QString(R"(
        QMainWindow {
            background-color: %1;
            color: %2;
        }
        QWidget {
            font-family: 'Segoe UI Variable', 'Segoe UI', sans-serif;
            font-size: 12px;
            color: %2;
        }
        QToolTip {
            background-color: %3;
            color: %2;
            border: 1px solid %4;
            border-radius: 4px;
            padding: 4px 8px;
            font-size: 11px;
        }
        QStatusBar {
            background-color: %5;
            color: %6;
            border-top: 1px solid %4;
            font-size: 11px;
        }
    )").arg(
        ThemeColors::MainBackground,
        ThemeColors::PrimaryText,
        ThemeColors::RaisedSurface,
        ThemeColors::Border,
        ThemeColors::SecondarySurface,
        ThemeColors::SecondaryText
    ) + scrollBarStyle() + splitterStyle();
}

QString Theme::emulatorStyle() {
    return QString(R"(
        QMainWindow#deviceEmulatorWindow, QWidget#deviceEmulatorContainer {
            background-color: %1;
        }
        #emulatorDeviceFrame {
            background-color: %2;
            border: 3px solid %3;
            border-radius: 38px;
        }
        #emulatorDynamicIsland {
            background-color: #000000;
            border: 1px solid #1A1D24;
            border-radius: 12px;
        }
        #emulatorHomeIndicator {
            background-color: #555E70;
            border-radius: 2px;
        }
        #emulatorSideBar {
            background-color: %4;
            border-left: 1px solid %5;
        }
    )").arg(
        ThemeColors::MainBackground,
        ThemeColors::DeviceChrome,
        ThemeColors::DeviceBezel,
        ThemeColors::SecondarySurface,
        ThemeColors::BorderSubtle
    );
}

QString Theme::commandBarStyle() {
    return QString(R"(
        #commandBar {
            background-color: %1;
            border-bottom: 1px solid %2;
            min-height: 44px;
            max-height: 44px;
        }
        #commandBar QLabel {
            font-size: 12px;
        }
        #projectTitleLabel {
            font-weight: 600;
            color: %3;
            font-size: 13px;
        }
        #projectSubLabel {
            color: %4;
            font-size: 11px;
        }
    )").arg(
        ThemeColors::SecondarySurface,
        ThemeColors::Border,
        ThemeColors::PrimaryText,
        ThemeColors::SecondaryText
    );
}

QString Theme::navRailStyle() {
    return QString(R"(
        #navRail {
            background-color: %1;
            border-right: 1px solid %2;
            min-width: 52px;
            max-width: 52px;
        }
        #navRail QPushButton {
            background-color: transparent;
            border: none;
            border-radius: 6px;
            color: %3;
            padding: 8px 0;
            font-size: 11px;
            font-weight: 500;
        }
        #navRail QPushButton:hover {
            background-color: %4;
            color: %5;
        }
        #navRail QPushButton:checked {
            background-color: %6;
            color: %7;
            border-left: 2px solid %7;
        }
    )").arg(
        ThemeColors::MainBackground,
        ThemeColors::Border,
        ThemeColors::SecondaryText,
        ThemeColors::SecondarySurface,
        ThemeColors::PrimaryText,
        ThemeColors::RaisedSurface,
        ThemeColors::PrimaryAccent
    );
}

QString Theme::cardStyle() {
    return QString(R"(
        QFrame.cardFrame, #cardFrame {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 8px;
            padding: 16px;
        }
    )").arg(ThemeColors::CardBackground, ThemeColors::BorderSubtle);
}

QString Theme::buttonStyle(bool isPrimary, bool isDestructive) {
    if (isDestructive) {
        return QString(R"(
            QPushButton {
                background-color: #8C2429;
                color: #FFFFFF;
                border: 1px solid #B03036;
                border-radius: 6px;
                padding: 6px 14px;
                font-size: 12px;
                font-weight: 500;
            }
            QPushButton:hover {
                background-color: #A32A30;
                border-color: #C23940;
            }
            QPushButton:pressed {
                background-color: #731E22;
            }
            QPushButton:disabled {
                background-color: #382428;
                color: #806065;
                border-color: #4A2A2E;
            }
        )");
    }

    if (isPrimary) {
        return QString(R"(
            QPushButton {
                background-color: %1;
                color: #FFFFFF;
                border: 1px solid #3584E4;
                border-radius: 6px;
                padding: 6px 16px;
                font-size: 12px;
                font-weight: 600;
            }
            QPushButton:hover {
                background-color: %2;
            }
            QPushButton:pressed {
                background-color: %3;
            }
            QPushButton:disabled {
                background-color: #213045;
                color: #556B88;
                border-color: #2C3E56;
            }
        )").arg(
            ThemeColors::PrimaryAccent,
            ThemeColors::PrimaryAccentHover,
            ThemeColors::PrimaryAccentPressed
        );
    }

    return QString(R"(
        QPushButton {
            background-color: %1;
            color: %2;
            border: 1px solid %3;
            border-radius: 6px;
            padding: 6px 14px;
            font-size: 12px;
            font-weight: 500;
        }
        QPushButton:hover {
            background-color: %4;
            border-color: #4A5364;
            color: #FFFFFF;
        }
        QPushButton:pressed {
            background-color: %1;
        }
        QPushButton:disabled {
            background-color: #1A1D24;
            color: #4E5666;
            border-color: #252A34;
        }
    )").arg(
        ThemeColors::SecondarySurface,
        ThemeColors::PrimaryText,
        ThemeColors::Border,
        ThemeColors::RaisedSurface
    );
}

QString Theme::iconButtonStyle() {
    return QString(R"(
        QPushButton {
            background-color: transparent;
            border: 1px solid transparent;
            border-radius: 6px;
            padding: 5px;
            color: %1;
        }
        QPushButton:hover {
            background-color: %2;
            border-color: %3;
            color: %4;
        }
        QPushButton:pressed {
            background-color: %5;
        }
        QPushButton:checked {
            background-color: %2;
            border-color: %6;
            color: %6;
        }
    )").arg(
        ThemeColors::SecondaryText,
        ThemeColors::RaisedSurface,
        ThemeColors::BorderSubtle,
        ThemeColors::PrimaryText,
        ThemeColors::SecondarySurface,
        ThemeColors::PrimaryAccent
    );
}

QString Theme::badgeStyle(const QString& bgHex, const QString& textHex) {
    return QString(R"(
        QLabel {
            background-color: %1;
            color: %2;
            border-radius: 4px;
            padding: 3px 8px;
            font-size: 11px;
            font-weight: 600;
        }
    )").arg(bgHex, textHex);
}

QString Theme::tabBarStyle() {
    return QString(R"(
        QTabWidget::pane {
            border: 1px solid %1;
            background-color: %2;
        }
        QTabBar::tab {
            background-color: %3;
            color: %4;
            padding: 7px 14px;
            border: 1px solid %1;
            border-bottom: none;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
            margin-right: 2px;
            font-size: 11px;
            font-weight: 500;
        }
        QTabBar::tab:hover {
            background-color: %5;
            color: %6;
        }
        QTabBar::tab:selected {
            background-color: %2;
            color: %7;
            border-bottom: 2px solid %7;
            font-weight: 600;
        }
    )").arg(
        ThemeColors::Border,
        ThemeColors::SecondarySurface,
        ThemeColors::MainBackground,
        ThemeColors::SecondaryText,
        ThemeColors::RaisedSurface,
        ThemeColors::PrimaryText,
        ThemeColors::PrimaryAccent
    );
}

QString Theme::splitterStyle() {
    return QString(R"(
        QSplitter::handle {
            background-color: %1;
        }
        QSplitter::handle:horizontal {
            width: 3px;
        }
        QSplitter::handle:vertical {
            height: 3px;
        }
        QSplitter::handle:hover {
            background-color: %2;
        }
    )").arg(ThemeColors::BorderSubtle, ThemeColors::PrimaryAccent);
}

QString Theme::scrollBarStyle() {
    return QString(R"(
        QScrollBar:vertical {
            background: %1;
            width: 8px;
            margin: 0px;
        }
        QScrollBar::handle:vertical {
            background: %2;
            min-height: 20px;
            border-radius: 4px;
        }
        QScrollBar::handle:vertical:hover {
            background: %3;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
        QScrollBar:horizontal {
            background: %1;
            height: 8px;
            margin: 0px;
        }
        QScrollBar::handle:horizontal {
            background: %2;
            min-width: 20px;
            border-radius: 4px;
        }
        QScrollBar::handle:horizontal:hover {
            background: %3;
        }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            width: 0px;
        }
    )").arg(
        ThemeColors::MainBackground,
        ThemeColors::Border,
        ThemeColors::SecondaryText
    );
}

QString Theme::lineEditStyle() {
    return QString(R"(
        QLineEdit {
            background-color: %1;
            color: %2;
            border: 1px solid %3;
            border-radius: 6px;
            padding: 6px 10px;
            font-size: 12px;
        }
        QLineEdit:focus {
            border: 1px solid %4;
        }
    )").arg(
        ThemeColors::RaisedSurface,
        ThemeColors::PrimaryText,
        ThemeColors::Border,
        ThemeColors::BorderFocus
    );
}

QString Theme::comboBoxStyle() {
    return QString(R"(
        QComboBox {
            background-color: %1;
            color: %2;
            border: 1px solid %3;
            border-radius: 6px;
            padding: 5px 10px;
            font-size: 12px;
            min-width: 120px;
        }
        QComboBox:hover {
            border-color: #4A5364;
        }
        QComboBox:focus {
            border-color: %4;
        }
        QComboBox::drop-down {
            border: none;
            width: 20px;
        }
        QComboBox QAbstractItemView {
            background-color: %1;
            color: %2;
            border: 1px solid %3;
            selection-background-color: %4;
            selection-color: #FFFFFF;
            padding: 4px;
        }
    )").arg(
        ThemeColors::SecondarySurface,
        ThemeColors::PrimaryText,
        ThemeColors::Border,
        ThemeColors::PrimaryAccent
    );
}

} // namespace ionexa::ui
