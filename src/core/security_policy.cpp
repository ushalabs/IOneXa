#include "security_policy.hpp"

#include <QRegularExpression>

namespace ionexa::core {

bool SecurityPolicy::isLoopbackHost(const QString& host) {
    QString h = host.trimmed().toLower();
    return h == "localhost" ||
           h == "127.0.0.1" ||
           h == "::1" ||
           h == "0.0.0.0" ||
           h == "[::1]";
}

bool SecurityPolicy::isDisallowedScheme(const QString& scheme) {
    QString s = scheme.trimmed().toLower();
    return s == "javascript" ||
           s == "vbscript" ||
           s == "file" ||
           s == "data" ||
           s == "res" ||
           s == "ms-appx";
}

bool SecurityPolicy::isSafePreviewUrl(const QString& urlString) {
    if (urlString.trimmed().isEmpty()) {
        return false;
    }

    if (urlString == "about:blank") {
        return true;
    }

    QUrl url(urlString);
    if (!url.isValid()) {
        return false;
    }

    QString scheme = url.scheme().toLower();
    if (isDisallowedScheme(scheme)) {
        return false;
    }

    if (scheme != "http" && scheme != "https") {
        return false;
    }

    // Must be loopback host to prevent navigating the WebView2 preview to arbitrary external web destinations
    if (!isLoopbackHost(url.host())) {
        return false;
    }

    // Must use an unprivileged port (Metro typically 8081-8099)
    if (url.port() != -1 && !isSafeDevPort(url.port())) {
        return false;
    }

    return true;
}

bool SecurityPolicy::containsTraversalSequences(const QString& path) {
    if (path.contains('\0')) {
        return true;
    }
    // Check for directory traversal sequences
    return path.contains("../") ||
           path.contains("..\\") ||
           path == ".." ||
           path.startsWith("../") ||
           path.startsWith("..\\");
}

bool SecurityPolicy::isPathWithinDirectory(const QString& basePath, const QString& candidatePath) {
    if (containsTraversalSequences(candidatePath)) {
        return false;
    }

    QDir baseDir(basePath);
    if (!baseDir.exists()) {
        return false;
    }

    QString canonicalBase = QDir::cleanPath(baseDir.canonicalPath());
    QFileInfo candidateInfo(candidatePath);

    // If candidate exists, use canonicalPath; otherwise resolve against basePath
    QString canonicalCandidate;
    if (candidateInfo.exists()) {
        canonicalCandidate = QDir::cleanPath(candidateInfo.canonicalFilePath());
    } else {
        canonicalCandidate = QDir::cleanPath(baseDir.filePath(candidatePath));
    }

#ifdef Q_OS_WIN
    Qt::CaseSensitivity cs = Qt::CaseInsensitive;
#else
    Qt::CaseSensitivity cs = Qt::CaseSensitive;
#endif

    if (canonicalCandidate.compare(canonicalBase, cs) == 0) {
        return true;
    }

    QString basePrefix = canonicalBase.endsWith('/') ? canonicalBase : canonicalBase + '/';
    return canonicalCandidate.startsWith(basePrefix, cs);
}

bool SecurityPolicy::isSafeDevPort(int port) {
    // Port must be in the unprivileged dynamic/private port range for dev servers (1024 - 65535)
    return port >= 1024 && port <= 65535;
}

QString SecurityPolicy::maskToken(const QString& token) {
    if (token.length() <= 8) {
        return "[REDACTED]";
    }
    return token.left(3) + "..." + token.right(3) + " [REDACTED]";
}

} // namespace ionexa::core
