#include "cachednetworkworker.h"
#include <QNetworkDiskCache>
#include <QStandardPaths>

CachedNetworkWorker::CachedNetworkWorker(QObject* parent)
    : QObject(parent), m_nam(new QNetworkAccessManager(this))
{
    QNetworkDiskCache* cache = new QNetworkDiskCache(this);
    cache->setCacheDirectory(
        QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/http/"));

    m_nam->setAutoDeleteReplies(true);
    m_nam->setCache(cache);
}

QNetworkReply* CachedNetworkWorker::get(const QUrl& url)
{
    QNetworkRequest req(url);
    req.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
    return m_nam->get(req);
}
