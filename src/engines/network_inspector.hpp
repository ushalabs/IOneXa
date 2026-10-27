#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <QMap>
#include <QDateTime>

namespace ionexa::engines {

enum class ResourceType {
    All = 0,
    Fetch,
    XHR,
    Script,
    Stylesheet,
    Image,
    Font,
    Document,
    Other
};

struct NetworkRequestEntry {
    qint64 id{0};
    QString url;
    QString method{"GET"};
    int statusCode{0};
    QString statusText;
    ResourceType resourceType{ResourceType::Fetch};
    qint64 startTimeMs{0};
    qint64 durationMs{0};
    qint64 responseBytes{0};
    QMap<QString, QString> requestHeaders;
    QMap<QString, QString> responseHeaders;
    QString requestBody;
    QString responseBody;
    bool isCompleted{false};
    bool isFailed{false};
    QString failureReason;
};

class NetworkInspector : public QObject {
    Q_OBJECT

public:
    explicit NetworkInspector(QObject* parent = nullptr);
    ~NetworkInspector() override = default;

    qint64 recordRequestStart(const QString& url,
                              const QString& method,
                              ResourceType type,
                              const QMap<QString, QString>& headers = {},
                              const QString& body = "");

    void recordResponse(qint64 requestId,
                        int statusCode,
                        const QString& statusText,
                        qint64 durationMs,
                        qint64 bytes = 0,
                        const QMap<QString, QString>& headers = {},
                        const QString& responseBody = "");

    void recordFailure(qint64 requestId,
                       const QString& errorReason,
                       qint64 durationMs);

    void clear();

    [[nodiscard]] const QVector<NetworkRequestEntry>& allRequests() const { return m_requests; }
    [[nodiscard]] QVector<NetworkRequestEntry> filteredRequests(ResourceType typeFilter = ResourceType::All,
                                                                int statusCodeFilter = 0,
                                                                const QString& searchUrl = "") const;

    [[nodiscard]] int totalCount() const { return m_requests.size(); }
    [[nodiscard]] int errorCount() const;
    [[nodiscard]] qint64 totalTransferredBytes() const;

    [[nodiscard]] QString exportAsHar() const;
    [[nodiscard]] QString exportAsJson() const;

    static QMap<QString, QString> redactHeaders(const QMap<QString, QString>& headers);
    static ResourceType deduceResourceType(const QString& url, const QString& mimeType = "");

signals:
    void requestStarted(const NetworkRequestEntry& request);
    void requestCompleted(const NetworkRequestEntry& request);
    void requestFailed(const NetworkRequestEntry& request);
    void cleared();

private:
    QVector<NetworkRequestEntry> m_requests;
    qint64 m_nextId{1};
    int m_maxEntries{2000};
};

} // namespace ionexa::engines
