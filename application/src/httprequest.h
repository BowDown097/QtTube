#pragma once
#include <QNetworkRequest>

class QNetworkAccessManager;
class QNetworkReply;

class HttpReply : public QObject
{
    Q_OBJECT
    friend class HttpRequest;
public:
    enum Operation
    {
        HeadOperation = 1,
        GetOperation,
        PutOperation,
        PostOperation,
        DeleteOperation,
        UnknownOperation = 0
    };

    // if you just want the QNetworkAccessManager that this uses, because it does have some extra goodies
    static QNetworkAccessManager* networkAccessManager();

    QString getFileName() const;
    QByteArray header(const QByteArray& key) const;
    QByteArray header(QNetworkRequest::KnownHeaders header) const;
    const QList<std::pair<QByteArray, QByteArray>>& headers() const;
    bool isSuccessful() const;
    const QByteArray& readAll() const;
    int statusCode() const;

    QByteArray requestHeader(const QByteArray& headerName) const;
    const QList<std::pair<QByteArray, QByteArray>>& requestHeaders() const { return m_requestHeaders; }
    const QUrl& url() const { return m_url; }
private:
    std::variant<QByteArray, QIODevice*> m_destination;
    QList<std::pair<QByteArray, QByteArray>> m_requestHeaders;
    QUrl m_url;

    explicit HttpReply(QList<std::pair<QByteArray, QByteArray>>&& requestHeaders, const QUrl& url, QIODevice* ioDevice)
        : m_destination(ioDevice ? std::variant<QByteArray, QIODevice*>(ioDevice)
                                 : std::variant<QByteArray, QIODevice*>(QByteArray())),
          m_requestHeaders(std::move(requestHeaders)),
          m_url(url) {}
private slots:
    void readyRead(QNetworkReply* networkReply);
signals:
    void finished(const HttpReply& request);
};

class HttpRequest
{
public:
    HttpRequest& withAttribute(QNetworkRequest::Attribute code, const QVariant& value)
    { m_attributes.emplaceBack(code, value); return *this; }
    HttpRequest& withDiskCache(bool usingDiskCache = true)
    { m_usingDiskCache = usingDiskCache; return *this; }
    HttpRequest& withHeader(const QByteArray& name, const QByteArray& value)
    { m_headers.emplaceBack(name, value); return *this; }
    HttpRequest& withHeaders(const QList<std::pair<QByteArray, QByteArray>>& headers)
    { m_headers = headers; return *this; }
    HttpRequest& writingToIODevice(QIODevice* ioDevice)
    { m_ioDevice = ioDevice; return *this; }

    HttpReply* request(const QUrl& url, HttpReply::Operation operation, const QByteArray& data = {});

    HttpReply* deleteResource(const QUrl& url) { return request(url, HttpReply::DeleteOperation); }
    HttpReply* get(const QUrl& url) { return request(url, HttpReply::GetOperation); }
    HttpReply* head(const QUrl& url) { return request(url, HttpReply::HeadOperation); }
    HttpReply* post(const QUrl& url, const QByteArray& data) { return request(url, HttpReply::PostOperation, data); }
    HttpReply* put(const QUrl& url, const QByteArray& data) { return request(url, HttpReply::PutOperation, data); }
private:
    QList<std::pair<QNetworkRequest::Attribute, QVariant>> m_attributes;
    QList<std::pair<QByteArray, QByteArray>> m_headers;
    QIODevice* m_ioDevice{};
    bool m_usingDiskCache{};

    QNetworkReply* networkReply(
        const QNetworkRequest& request, HttpReply::Operation operation, const QByteArray& data = {});
};
