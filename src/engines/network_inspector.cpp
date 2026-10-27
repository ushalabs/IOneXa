#include "network_inspector.hpp"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <algorithm>

namespace ionexa::engines {

NetworkInspector::NetworkInspector(QObject* parent)
    : QObject(parent) {}

QMap<QString, QString> NetworkInspector::redactHeaders(const QMap<QString, QString>& headers) {
    static const QStringList sensitiveKeys = {
        "authorization",
        "proxy-authorization",
        "cookie",
        "set-cookie",
        "x-api-key",
        "api-key",
        "token",
        "secret",
        "x-auth-token"
    };

    QMap<QString, QString> result;
    for (auto it = headers.begin(); it != headers.end(); ++it) {
        QString lowerKey = it.key().trimmed().toLower();
        if (sensitiveKeys.contains(lowerKey)) {
            if (lowerKey == "authorization" && it.value().startsWith("Bearer ", Qt::CaseInsensitive)) {
                result[it.key()] = "Bearer [REDACTED]";
            } else {
                result[it.key()] = "[REDACTED]";
            }
        } else {
            result[it.key()] = it.value();
        }
    }
    return result;
}

ResourceType NetworkInspector::deduceResourceType(const QString& url, const QString& mimeType) {
    QString lowerUrl = url.toLower();
    QString lowerMime = mimeType.toLower();

    if (lowerMime.contains("javascript") || lowerUrl.endsWith(".js") || lowerUrl.contains(".bundle")) {
        return ResourceType::Script;
    }
    if (lowerMime.contains("css") || lowerUrl.endsWith(".css")) {
        return ResourceType::Stylesheet;
    }
    if (lowerMime.contains("image") || lowerUrl.endsWith(".png") || lowerUrl.endsWith(".jpg") || lowerUrl.endsWith(".jpeg") || lowerUrl.endsWith(".webp") || lowerUrl.endsWith(".svg") || lowerUrl.endsWith(".ico")) {
        return ResourceType::Image;
    }
    if (lowerMime.contains("font") || lowerUrl.endsWith(".woff") || lowerUrl.endsWith(".woff2") || lowerUrl.endsWith(".ttf") || lowerUrl.endsWith(".otf")) {
        return ResourceType::Font;
    }
    if (lowerMime.contains("html") || lowerUrl.endsWith(".html") || lowerUrl.endsWith("/")) {
        return ResourceType::Document;
    }
    if (lowerMime.contains("json") || lowerUrl.contains("/api/")) {
        return ResourceType::Fetch;
    }

    return ResourceType::Other;
}

qint64 NetworkInspector::recordRequestStart(const QString& url,
                                            const QString& method,
                                            ResourceType type,
                                            const QMap<QString, QString>& headers,
                                            const QString& body) {
    NetworkRequestEntry entry;
    entry.id = m_nextId++;
    entry.url = url;
    entry.method = method.toUpper();
    entry.resourceType = type;
    entry.startTimeMs = QDateTime::currentMSecsSinceEpoch();
    entry.requestHeaders = redactHeaders(headers);
    entry.requestBody = body;

    m_requests.push_back(entry);
    if (m_requests.size() > m_maxEntries) {
        m_requests.removeFirst();
    }

    emit requestStarted(entry);
    return entry.id;
}

void NetworkInspector::recordResponse(qint64 requestId,
                                     int statusCode,
                                     const QString& statusText,
                                     qint64 durationMs,
                                     qint64 bytes,
                                     const QMap<QString, QString>& headers,
                                     const QString& responseBody) {
    for (auto& req : m_requests) {
        if (req.id == requestId) {
            req.statusCode = statusCode;
            req.statusText = statusText;
            req.durationMs = durationMs;
            req.responseBytes = bytes;
            req.responseHeaders = redactHeaders(headers);
            req.responseBody = responseBody;
            req.isCompleted = true;
            req.isFailed = (statusCode >= 400);

            emit requestCompleted(req);
            return;
        }
    }
}

void NetworkInspector::recordFailure(qint64 requestId,
                                    const QString& errorReason,
                                    qint64 durationMs) {
    for (auto& req : m_requests) {
        if (req.id == requestId) {
            req.durationMs = durationMs;
            req.isCompleted = true;
            req.isFailed = true;
            req.failureReason = errorReason;

            emit requestFailed(req);
            return;
        }
    }
}

void NetworkInspector::clear() {
    m_requests.clear();
    emit cleared();
}

int NetworkInspector::errorCount() const {
    int errors = 0;
    for (const auto& req : m_requests) {
        if (req.isFailed || req.statusCode >= 400) {
            errors++;
        }
    }
    return errors;
}

qint64 NetworkInspector::totalTransferredBytes() const {
    qint64 bytes = 0;
    for (const auto& req : m_requests) {
        bytes += req.responseBytes;
    }
    return bytes;
}

QVector<NetworkRequestEntry> NetworkInspector::filteredRequests(ResourceType typeFilter,
                                                              int statusCodeFilter,
                                                              const QString& searchUrl) const {
    QVector<NetworkRequestEntry> result;
    for (const auto& req : m_requests) {
        if (typeFilter != ResourceType::All && req.resourceType != typeFilter) {
            continue;
        }
        if (statusCodeFilter > 0) {
            if (statusCodeFilter == 400 && (req.statusCode < 400 || req.statusCode >= 500)) continue;
            if (statusCodeFilter == 500 && req.statusCode < 500) continue;
            if (statusCodeFilter == 200 && (req.statusCode < 200 || req.statusCode >= 300)) continue;
        }
        if (!searchUrl.isEmpty() && !req.url.contains(searchUrl, Qt::CaseInsensitive)) {
            continue;
        }
        result.push_back(req);
    }
    return result;
}

QString NetworkInspector::exportAsJson() const {
    QJsonArray array;
    for (const auto& req : m_requests) {
        QJsonObject obj;
        obj["id"] = req.id;
        obj["url"] = req.url;
        obj["method"] = req.method;
        obj["statusCode"] = req.statusCode;
        obj["statusText"] = req.statusText;
        obj["resourceType"] = static_cast<int>(req.resourceType);
        obj["durationMs"] = req.durationMs;
        obj["responseBytes"] = req.responseBytes;
        obj["isFailed"] = req.isFailed;

        QJsonObject reqHeaders;
        for (auto it = req.requestHeaders.begin(); it != req.requestHeaders.end(); ++it) {
            reqHeaders[it.key()] = it.value();
        }
        obj["requestHeaders"] = reqHeaders;

        QJsonObject resHeaders;
        for (auto it = req.responseHeaders.begin(); it != req.responseHeaders.end(); ++it) {
            resHeaders[it.key()] = it.value();
        }
        obj["responseHeaders"] = resHeaders;

        if (!req.requestBody.isEmpty()) obj["requestBody"] = req.requestBody;
        if (!req.responseBody.isEmpty()) obj["responseBody"] = req.responseBody;
        if (!req.failureReason.isEmpty()) obj["failureReason"] = req.failureReason;

        array.append(obj);
    }
    QJsonDocument doc(array);
    return doc.toJson(QJsonDocument::Indented);
}

QString NetworkInspector::exportAsHar() const {
    QJsonObject logObj;
    logObj["version"] = "1.2";

    QJsonObject creatorObj;
    creatorObj["name"] = "IOneXa Network Inspector";
    creatorObj["version"] = "0.1.0";
    logObj["creator"] = creatorObj;

    QJsonArray entriesArray;
    for (const auto& req : m_requests) {
        QJsonObject entry;
        entry["startedDateTime"] = QDateTime::fromMSecsSinceEpoch(req.startTimeMs).toString(Qt::ISODateWithMs);
        entry["time"] = req.durationMs;

        // Request
        QJsonObject reqObj;
        reqObj["method"] = req.method;
        reqObj["url"] = req.url;
        reqObj["httpVersion"] = "HTTP/1.1";

        QJsonArray reqHeadersArr;
        for (auto it = req.requestHeaders.begin(); it != req.requestHeaders.end(); ++it) {
            QJsonObject h;
            h["name"] = it.key();
            h["value"] = it.value();
            reqHeadersArr.append(h);
        }
        reqObj["headers"] = reqHeadersArr;
        reqObj["headersSize"] = -1;
        reqObj["bodySize"] = req.requestBody.toUtf8().size();
        entry["request"] = reqObj;

        // Response
        QJsonObject resObj;
        resObj["status"] = req.statusCode;
        resObj["statusText"] = req.statusText;
        resObj["httpVersion"] = "HTTP/1.1";

        QJsonArray resHeadersArr;
        for (auto it = req.responseHeaders.begin(); it != req.responseHeaders.end(); ++it) {
            QJsonObject h;
            h["name"] = it.key();
            h["value"] = it.value();
            resHeadersArr.append(h);
        }
        resObj["headers"] = resHeadersArr;
        resObj["headersSize"] = -1;
        resObj["bodySize"] = req.responseBytes;

        QJsonObject contentObj;
        contentObj["size"] = req.responseBytes;
        contentObj["mimeType"] = req.responseHeaders.value("content-type", "application/octet-stream");
        if (!req.responseBody.isEmpty()) {
            contentObj["text"] = req.responseBody;
        }
        resObj["content"] = contentObj;
        entry["response"] = resObj;

        // Cache & timings
        entry["cache"] = QJsonObject{};
        QJsonObject timingsObj;
        timingsObj["send"] = 0;
        timingsObj["wait"] = req.durationMs;
        timingsObj["receive"] = 0;
        entry["timings"] = timingsObj;

        entriesArray.append(entry);
    }

    logObj["entries"] = entriesArray;

    QJsonObject root;
    root["log"] = logObj;

    QJsonDocument doc(root);
    return doc.toJson(QJsonDocument::Indented);
}

} // namespace ionexa::engines
