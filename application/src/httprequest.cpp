#include "httprequest.h"
#include <QNetworkDiskCache>
#include <QNetworkReply>
#include <QStandardPaths>

QByteArray HttpReply::header(const QByteArray& key) const
{
    if (QNetworkReply* reply = qobject_cast<QNetworkReply*>(parent()))
        return reply->rawHeader(key);
    else
        throw std::runtime_error("Attempting to access reply when request has not been made");
}

QByteArray HttpReply::header(QNetworkRequest::KnownHeaders header) const
{
    if (QNetworkReply* reply = qobject_cast<QNetworkReply*>(parent()))
        return reply->header(header).toByteArray();
    else
        throw std::runtime_error("Attempting to access reply when request has not been made");
}

const QList<std::pair<QByteArray, QByteArray>>& HttpReply::headers() const
{
    if (QNetworkReply* reply = qobject_cast<QNetworkReply*>(parent()))
        return reply->rawHeaderPairs();
    else
        throw std::runtime_error("Attempting to access reply when request has not been made");
}

bool HttpReply::isSuccessful() const
{
    int status = statusCode();
    return status >= 200 && status < 300;
}

QNetworkAccessManager* HttpReply::networkAccessManager()
{
    static thread_local QNetworkAccessManager* nam = [] {
        QNetworkAccessManager* nam = new QNetworkAccessManager;
        nam->setAutoDeleteReplies(true);
        nam->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);

        QNetworkDiskCache* diskCache = new QNetworkDiskCache(nam);
        diskCache->setCacheDirectory(
            QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/http/"));
        nam->setCache(diskCache);

        return nam;
    }();

    return nam;
}

QByteArray HttpReply::readAll() const
{
    if (QNetworkReply* reply = qobject_cast<QNetworkReply*>(parent()))
        return reply->readAll();
    else
        throw std::runtime_error("Attempting to access reply when request has not been made");
}

QByteArray HttpReply::requestHeader(const QByteArray& headerName) const
{
    QByteArray result;
    const char* separator = "";

    for (const auto& [name, value] : m_requestHeaders)
    {
        if (name.compare(headerName, Qt::CaseInsensitive) == 0)
        {
            result.append(separator);
            result.append(value);
            separator = "\n";
        }
    }

    return result;
}

int HttpReply::statusCode() const
{
    if (QNetworkReply* reply = qobject_cast<QNetworkReply*>(parent()))
        return reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    else
        throw std::runtime_error("Attempting to access reply when request has not been made");
}

QNetworkReply* HttpRequest::networkReply(
    const QNetworkRequest& request, HttpReply::Operation operation, const QByteArray& data)
{
    switch (operation)
    {
    case HttpReply::HeadOperation:
        return HttpReply::networkAccessManager()->head(request);
    case HttpReply::GetOperation:
        return HttpReply::networkAccessManager()->get(request);
    case HttpReply::PutOperation:
        return HttpReply::networkAccessManager()->put(request, data);
    case HttpReply::PostOperation:
        return HttpReply::networkAccessManager()->post(request, data);
    case HttpReply::DeleteOperation:
        return HttpReply::networkAccessManager()->deleteResource(request);
    default:
        throw std::runtime_error("Making request with invalid operation (value: " + std::to_string(operation));
    }
}

HttpReply* HttpRequest::request(
    const QUrl& url, HttpReply::Operation operation, const QByteArray& data)
{
    HttpReply* result = new HttpReply(std::move(m_headers), url);

    QNetworkRequest req(result->m_url);
    req.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
    req.setAttribute(QNetworkRequest::CacheSaveControlAttribute, m_usingDiskCache);

    for (const auto& [code, value] : std::as_const(m_attributes))
        req.setAttribute(code, value);
    for (const auto& [name, value] : std::as_const(result->m_requestHeaders))
        req.setRawHeader(name, value);

    if (QNetworkReply* reply = networkReply(req, operation, data))
    {
        result->setParent(reply);
        QObject::connect(reply, &QNetworkReply::finished, result, [result] { emit result->finished(*result); });
    }

    return result;
}
