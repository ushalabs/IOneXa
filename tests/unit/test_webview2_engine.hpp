#pragma once

#include <QTest>
#include "engines/webview2_widget.hpp"

using namespace ionexa::engines;

class TestWebView2Engine : public QObject {
    Q_OBJECT

private slots:
    void testWidgetCreationAndAttributes() {
        WebView2Widget widget;
        QVERIFY(widget.testAttribute(Qt::WA_NativeWindow));
        QVERIFY(widget.testAttribute(Qt::WA_DontCreateNativeAncestors));
        QCOMPARE(widget.currentUrl(), QString(""));
        QVERIFY(!widget.isInitialized());
    }

    void testZoomAndReloadSafety() {
        WebView2Widget widget;
        // Verify setting zoom or reloading before initialization does not crash
        widget.setZoomFactor(1.5);
        widget.reload();
        widget.executeScript("console.log('test')");
        QVERIFY(true);
    }
};
