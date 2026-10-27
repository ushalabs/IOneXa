#pragma once

#include <QTest>
#include <QObject>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QUrl>
#include <QUrlQuery>
#include <QRegularExpression>
#include <QJsonObject>
#include <QJsonDocument>

#include "project/compatibility_analyzer.hpp"
#include "project/project_scanner.hpp"

class TestFeatureSuite : public QObject {
    Q_OBJECT

private slots:
    void testExpoBasicStructureAndAssets();
    void testExpoRouterFileBasedRoutes();
    void testNavigationAndParamResolution();
    void testFormValidationLifecycle();
    void testLocalPersistenceContract();
    void testAssetBundlingVerification();
    void testFeatureCompatibilityMatrix();

private:
    QString findFixturesDir();
};

inline QString TestFeatureSuite::findFixturesDir() {
    QStringList candidates = {
        QDir::current().filePath("tests/fixtures"),
        QDir::current().filePath("../tests/fixtures"),
        QDir::current().filePath("../../tests/fixtures")
    };
    for (const QString& candidate : candidates) {
        if (QDir(candidate).exists()) {
            return QDir(candidate).canonicalPath();
        }
    }
    return QString();
}

inline void TestFeatureSuite::testExpoBasicStructureAndAssets() {
    QString fixturesDir = findFixturesDir();
    QVERIFY2(!fixturesDir.isEmpty(), "Fixtures directory not found");

    QString basicPath = QDir(fixturesDir).filePath("expo-basic");
    QVERIFY(QDir(basicPath).exists());

    // Verify critical project files
    QVERIFY(QFile::exists(QDir(basicPath).filePath("package.json")));
    QVERIFY(QFile::exists(QDir(basicPath).filePath("app.json")));
    QVERIFY(QFile::exists(QDir(basicPath).filePath("App.js")));

    // Verify assets directory and images
    QString assetsPath = QDir(basicPath).filePath("assets");
    QVERIFY(QDir(assetsPath).exists());

    QString iconFile = QDir(assetsPath).filePath("icon.png");
    QString splashFile = QDir(assetsPath).filePath("splash-icon.png");
    QString faviconFile = QDir(assetsPath).filePath("favicon.png");

    QVERIFY(QFile::exists(iconFile));
    QVERIFY(QFile::exists(splashFile));
    QVERIFY(QFile::exists(faviconFile));

    // Verify image loading and dimensions
    QImage iconImg(iconFile);
    QVERIFY(!iconImg.isNull());
    QVERIFY(iconImg.width() > 0 && iconImg.height() > 0);

    QImage splashImg(splashFile);
    QVERIFY(!splashImg.isNull());
    QVERIFY(splashImg.width() > 0 && splashImg.height() > 0);
}

inline void TestFeatureSuite::testExpoRouterFileBasedRoutes() {
    QString fixturesDir = findFixturesDir();
    QVERIFY2(!fixturesDir.isEmpty(), "Fixtures directory not found");

    QString routerPath = QDir(fixturesDir).filePath("expo-router");
    QVERIFY(QDir(routerPath).exists());

    QString appDir = QDir(routerPath).filePath("app");
    QVERIFY(QDir(appDir).exists());

    // Verify all route components exist
    QStringList expectedRoutes = {
        "_layout.js",
        "index.js",
        "details.js",
        "form.js",
        "storage.js"
    };

    for (const QString& routeFile : expectedRoutes) {
        QString fullPath = QDir(appDir).filePath(routeFile);
        QVERIFY2(QFile::exists(fullPath), qPrintable(QString("Missing route file: %1").arg(routeFile)));

        QFile file(fullPath);
        QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
        QString content = QString::fromUtf8(file.readAll());
        file.close();

        // Check that component exports a default React function
        QVERIFY(content.contains("export default function"));
    }
}

inline void TestFeatureSuite::testNavigationAndParamResolution() {
    // Simulate Expo Router / Web navigation route resolution
    QUrl testUrl("exp://localhost:8081/details?item=WorkbenchFeature&mode=dark");
    QCOMPARE(testUrl.path(), QString("/details"));

    QUrlQuery query(testUrl.query());
    QCOMPARE(query.queryItemValue("item"), QString("WorkbenchFeature"));
    QCOMPARE(query.queryItemValue("mode"), QString("dark"));

    // Simulate Route Stack Push & Pop
    QStringList navigationStack;
    navigationStack.append("/");
    QCOMPARE(navigationStack.last(), QString("/"));

    navigationStack.append("/details");
    QCOMPARE(navigationStack.last(), QString("/details"));
    QCOMPARE(navigationStack.size(), 2);

    navigationStack.removeLast(); // Back action
    QCOMPARE(navigationStack.last(), QString("/"));
    QCOMPARE(navigationStack.size(), 1);
}

inline void TestFeatureSuite::testFormValidationLifecycle() {
    // Form validator model representing the FormScreen logic
    struct FormModel {
        QString name;
        QString email;
        bool agreeTerms{false};

        struct Result {
            bool valid;
            QString error;
        };

        Result validate() const {
            if (name.trimmed().isEmpty()) {
                return {false, "Name is required"};
            }
            if (!email.contains('@')) {
                return {false, "Valid email is required"};
            }
            if (!agreeTerms) {
                return {false, "Must agree to terms"};
            }
            return {true, ""};
        }
    };

    FormModel form;
    // Step 1: Initial empty state
    auto r1 = form.validate();
    QVERIFY(!r1.valid);
    QCOMPARE(r1.error, QString("Name is required"));

    // Step 2: Name filled, missing valid email
    form.name = "Usha Developer";
    auto r2 = form.validate();
    QVERIFY(!r2.valid);
    QCOMPARE(r2.error, QString("Valid email is required"));

    // Step 3: Invalid email without @
    form.email = "invalid-email";
    auto r3 = form.validate();
    QVERIFY(!r3.valid);
    QCOMPARE(r3.error, QString("Valid email is required"));

    // Step 4: Valid email, terms unchecked
    form.email = "dev@ushalabs.com";
    auto r4 = form.validate();
    QVERIFY(!r4.valid);
    QCOMPARE(r4.error, QString("Must agree to terms"));

    // Step 5: Terms accepted -> Successful validation
    form.agreeTerms = true;
    auto r5 = form.validate();
    QVERIFY(r5.valid);
    QVERIFY(r5.error.isEmpty());
}

inline void TestFeatureSuite::testLocalPersistenceContract() {
    // In-memory web localStorage simulator
    QMap<QString, QString> mockLocalStorage;

    auto setItem = [&](const QString& key, const QString& val) {
        mockLocalStorage[key] = val;
    };
    auto getItem = [&](const QString& key) -> QString {
        return mockLocalStorage.value(key, QString());
    };
    auto removeItem = [&](const QString& key) {
        mockLocalStorage.remove(key);
    };

    // Test simple string persistence
    setItem("app_theme", "dark");
    QCOMPARE(getItem("app_theme"), QString("dark"));

    // Test JSON object persistence
    QJsonObject userState;
    userState["username"] = "tester";
    userState["clicks"] = 42;
    QJsonDocument doc(userState);
    setItem("user_state", QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));

    QString retrievedRaw = getItem("user_state");
    QVERIFY(!retrievedRaw.isEmpty());

    QJsonDocument parsedDoc = QJsonDocument::fromJson(retrievedRaw.toUtf8());
    QVERIFY(parsedDoc.isObject());
    QCOMPARE(parsedDoc.object()["username"].toString(), QString("tester"));
    QCOMPARE(parsedDoc.object()["clicks"].toInt(), 42);

    // Test removal
    removeItem("app_theme");
    QVERIFY(getItem("app_theme").isEmpty());
    QCOMPARE(mockLocalStorage.size(), 1);
}

inline void TestFeatureSuite::testAssetBundlingVerification() {
    QString fixturesDir = findFixturesDir();
    QVERIFY2(!fixturesDir.isEmpty(), "Fixtures directory not found");

    QString basicPath = QDir(fixturesDir).filePath("expo-basic");
    QString appJsPath = QDir(basicPath).filePath("App.js");

    QFile file(appJsPath);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QString code = QString::fromUtf8(file.readAll());
    file.close();

    // Regex finding require('./assets/...')
    QRegularExpression assetRegex("require\\(['\"](\\./assets/[^'\"]+)['\"]\\)");
    auto matchIterator = assetRegex.globalMatch(code);

    int matchCount = 0;
    while (matchIterator.hasNext()) {
        auto match = matchIterator.next();
        QString relPath = match.captured(1);
        matchCount++;

        // Verify resolved asset path on disk exists
        QString absAssetPath = QDir(basicPath).filePath(relPath);
        QVERIFY2(QFile::exists(absAssetPath), qPrintable(QString("Asset does not exist: %1").arg(absAssetPath)));
    }
    QVERIFY(matchCount > 0);
}

inline void TestFeatureSuite::testFeatureCompatibilityMatrix() {
    QString fixturesDir = findFixturesDir();
    QVERIFY2(!fixturesDir.isEmpty(), "Fixtures directory not found");

    // Test expo-basic compatibility
    QString basicPath = QDir(fixturesDir).filePath("expo-basic");
    auto resBasic = ionexa::project::CompatibilityAnalyzer::analyze(basicPath);
    QVERIFY(resBasic.totalModulesAnalyzed > 0);
    QVERIFY(resBasic.compatibilityScore >= 80.0);
    QCOMPARE(resBasic.unsupportedCount, 0);

    // Test expo-router compatibility
    QString routerPath = QDir(fixturesDir).filePath("expo-router");
    auto resRouter = ionexa::project::CompatibilityAnalyzer::analyze(routerPath);
    QVERIFY(resRouter.totalModulesAnalyzed > 0);
    QVERIFY(resRouter.compatibilityScore >= 80.0);
    QCOMPARE(resRouter.unsupportedCount, 0);
}
