#pragma once
#include <QNetworkReply>

class CachedNetworkWorker : public QObject
{
public:
    explicit CachedNetworkWorker(QObject* parent = nullptr);
    Q_INVOKABLE QNetworkReply* get(const QUrl& url);
private:
    QNetworkAccessManager* m_nam;
};
