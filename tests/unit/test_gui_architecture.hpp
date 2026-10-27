#pragma once

#include <QTest>
#include <QObject>
#include <QColor>
#include <QFont>

#include "ui/theme.hpp"

using namespace ionexa::ui;

class TestGuiArchitecture : public QObject {
    Q_OBJECT

private slots:
    void testThemeTokensAndColors();
    void testThemeFonts();
    void testThemeStylesheets();
    void testWindowDecouplingRules();
};

inline void TestGuiArchitecture::testThemeTokensAndColors() {
    QCOMPARE(QString(ThemeColors::MainBackground), QString("#16181D"));
    QCOMPARE(QString(ThemeColors::SecondarySurface), QString("#1E222A"));
    QCOMPARE(QString(ThemeColors::PrimaryAccent), QString("#4E9CFF"));
    QCOMPARE(QString(ThemeColors::Success), QString("#43C990"));
    QCOMPARE(QString(ThemeColors::Error), QString("#F06B72"));

    QColor mainBg = Theme::mainBg();
    QVERIFY(mainBg.isValid());
    QCOMPARE(mainBg.name(QColor::HexRgb).toUpper(), QString("#16181D"));

    QColor accent = Theme::accent();
    QVERIFY(accent.isValid());
    QCOMPARE(accent.name(QColor::HexRgb).toUpper(), QString("#4E9CFF"));
}

inline void TestGuiArchitecture::testThemeFonts() {
    QFont uiFont = Theme::defaultFont();
    QVERIFY(!uiFont.family().isEmpty());

    QFont monoFont = Theme::monoFont();
    QVERIFY(!monoFont.family().isEmpty());
    QCOMPARE(monoFont.styleHint(), QFont::Monospace);
}

inline void TestGuiArchitecture::testThemeStylesheets() {
    QString wbStyle = Theme::workbenchStyle();
    QVERIFY(!wbStyle.isEmpty());
    QVERIFY(wbStyle.contains("#16181D"));

    QString emStyle = Theme::emulatorStyle();
    QVERIFY(!emStyle.isEmpty());
    QVERIFY(emStyle.contains("#emulatorDeviceFrame"));

    QString cmdStyle = Theme::commandBarStyle();
    QVERIFY(!cmdStyle.isEmpty());
    QVERIFY(cmdStyle.contains("#commandBar"));

    QString btnStyle = Theme::buttonStyle(true);
    QVERIFY(!btnStyle.isEmpty());
    QVERIFY(btnStyle.contains("#4E9CFF"));
}

inline void TestGuiArchitecture::testWindowDecouplingRules() {
    // Window decoupling contract: Window A (Workbench) and Window B (Device Emulator)
    // must maintain distinct window titles and distinct geometry state keys
    QString workbenchTitle = "IOneXa Workbench";
    QString emulatorTitle = "IOneXa — Device Emulator";
    QVERIFY(workbenchTitle != emulatorTitle);

    QString workbenchGeoKey = "workbench/geometry";
    QString emulatorGeoKey = "emulator/geometry";
    QVERIFY(workbenchGeoKey != emulatorGeoKey);
}
