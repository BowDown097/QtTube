#include "httprequest.h"
#include <QNetworkDiskCache>
#include <QNetworkReply>
#include <QStandardPaths>

// https://tools.ietf.org/html/rfc6266
QString HttpReply::getFileName() const
{
    if (QNetworkReply* reply = qobject_cast<QNetworkReply*>(parent());
        reply && reply->hasRawHeader("Content-Disposition"))
    {
        QString fileName;

        QByteArray disposition = reply->rawHeader("Content-Disposition");
        constexpr std::array keys = { QLatin1String("filename="), QLatin1String("filename*=UTF-8''") };

        for (const QLatin1String& key : keys)
        {
            if (int index = disposition.indexOf(key); index >= 0)
            {
                disposition = disposition.mid(index + key.size());
                if (disposition.startsWith('"') || disposition.startsWith('\''))
                    disposition = disposition.mid(1);
                if ((index = disposition.lastIndexOf('"')) > 0)
                    disposition = disposition.left(index);
                else if ((index = disposition.lastIndexOf('\'')) > 0)
                    disposition = disposition.left(index);
                fileName = QUrl::fromPercentEncoding(disposition);
            }
        }

        return fileName;
    }

    return {};
}

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

const QByteArray& HttpReply::readAll() const
{
    if (const QByteArray* byteArray = std::get_if<QByteArray>(&m_destination))
        return *byteArray;
    throw std::runtime_error("Attempting to get data from reply which has been written to an IO device");
}

void HttpReply::readyRead(QNetworkReply* networkReply)
{
    if (QByteArray* byteArray = std::get_if<QByteArray>(&m_destination))
        byteArray->append(networkReply->readAll());
    else if (QIODevice** ioDevice = std::get_if<QIODevice*>(&m_destination))
        (*ioDevice)->write(networkReply->readAll());
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
    HttpReply* result = new HttpReply(std::move(m_headers), url, m_ioDevice);

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
        QObject::connect(reply, &QNetworkReply::readyRead, result, [reply, result] { emit result->readyRead(reply); });
    }

    return result;
}
