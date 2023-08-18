#include "cachedhttp.h"
#include "localcache.h"
#include <QtNetwork>

CachedHttpReply::CachedHttpReply(const QByteArray &body, const QUrl &url, bool autoSignals)
    : bytes(body), requestUrl(url) {
    if (autoSignals) QTimer::singleShot(0, this, SLOT(emitSignals()));
}

QByteArray CachedHttpReply::body() const {
    return bytes;
}

void CachedHttpReply::emitSignals() {
    emit data(body());
    emit finished(*this);
    deleteLater();
}

WrappedHttpReply::WrappedHttpReply(CachedHttp &cachedHttp,
                                   LocalCache *cache,
                                   const QByteArray &key,
                                   HttpReply *httpReply)
    : HttpReply(httpReply), cachedHttp(cachedHttp), cache(cache), key(key), httpReply(httpReply) {
    connect(httpReply, &HttpReply::finished, this, &WrappedHttpReply::originFinished);
}

void WrappedHttpReply::originFinished(const HttpReply &reply) {
#ifdef HTTP_DEBUG
    qDebug() << reply.statusCode() << reply.url();
#endif
    bool success = reply.isSuccessful();
    if (!success) {
        // Fallback to stale cached data on HTTP error
        const QByteArray value = cache->possiblyStaleValue(key);
        if (!value.isNull()) {
#ifdef HTTP_DEBUG
            qDebug() << "Using stale cache value" << reply.url();
#endif
            emit data(value);
            auto replyFromCache = new CachedHttpReply(value, reply.url(), false);
            emit finished(*replyFromCache);
            replyFromCache->deleteLater();
            return;
        }
    }

    bool doCache = success;
    if (doCache) {
        const auto &validators = cachedHttp.getValidators();
        if (!validators.isEmpty()) {
            const QByteArray &mime = reply.header("Content-Type");
            auto i = validators.constFind(mime);
            if (i != validators.constEnd()) {
                auto validator = i.value();
                doCache = validator(reply);
            } else {
                i = validators.constFind("*");
                if (i != validators.constEnd()) {
                    auto validator = i.value();
                    doCache = validator(reply);
                }
            }
        }
    }

    if (doCache)
        cache->insert(key, reply.body());
#ifdef HTTP_DEBUG
    else
        qDebug() << "Not caching" << reply.statusCode() << reply.url();
#endif

    if (success) {
        emit data(reply.body());
    } else {
        emit error(reply.reasonPhrase());
    }
    emit finished(reply);
}

CachedHttp::CachedHttp(Http &http, const char *name)
    : http(http), cache(LocalCache::instance(name)), cachePostRequests(false) {}

void CachedHttp::setMaxSeconds(uint seconds) {
    cache->setMaxSeconds(seconds);
}

void CachedHttp::setMaxSize(uint maxSize) {
    cache->setMaxSize(maxSize);
}

HttpReply *CachedHttp::request(const HttpRequest &req) {
    bool cacheable = req.operation == QNetworkAccessManager::GetOperation ||
                     (cachePostRequests && req.operation == QNetworkAccessManager::PostOperation);
    if (!cacheable) {
#ifdef HTTP_DEBUG
        qDebug() << "Not cacheable" << req.url;
#endif
        return http.request(req);
    }
    const QByteArray key = requestHash(req);
    const QByteArray value = cache->value(key);
    if (!value.isNull()) {
        return new CachedHttpReply(value, req.url);
    }
    return new WrappedHttpReply(*this, cache, key, http.request(req));
}

QByteArray CachedHttp::requestHash(const HttpRequest &req) {
    const char sep = '|';

    QByteArray s;
    if (ignoreHostname) {
        s = (req.url.scheme() + sep + req.url.path() + sep + req.url.query()).toUtf8();
    } else {
        s = req.url.toEncoded();
    }
    s += sep + req.body + sep + QByteArray::number(req.offset);
    if (req.operation == QNetworkAccessManager::PostOperation) {
        s.append(sep);
        s.append("POST");
    }
    return LocalCache::hash(s);
}
