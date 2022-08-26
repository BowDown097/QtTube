#ifndef DOWNLOADMANAGER_HPP
#define DOWNLOADMANAGER_HPP
#include <QElapsedTimer>
#include <QQueue>
#include <QTimer>
#include <QUrl>
#include <QtNetwork>

class DownloadManager : public QObject
{
    Q_OBJECT
    QNetworkReply* currentReply = nullptr;
    QUrl currentUrl;
    QByteArray downloadData;
    QNetworkAccessManager manager;
    QQueue<QUrl> queue;
public:
    static DownloadManager& instance()
    {
        static DownloadManager dm;
        return dm;
    }

    DownloadManager(QObject* parent = nullptr) : QObject(parent) {}

    void append(const QUrl& url)
    {
        if (queue.isEmpty() && !currentReply)
            QTimer::singleShot(0, this, &DownloadManager::startNextDownload);

        queue.enqueue(url);
    }
signals:
    void finishedDownload(const QByteArray& data);
private slots:
    void downloadFinished()
    {
        currentReply = nullptr;
        emit finishedDownload(downloadData);
        startNextDownload();
    }

    void downloadReadyRead() { downloadData.append(currentReply->readAll()); }

    void startNextDownload()
    {
        if (queue.isEmpty())
            return;

        currentUrl = queue.dequeue();
        QNetworkRequest request(currentUrl);
        currentReply = manager.get(request);
        connect(currentReply, &QNetworkReply::finished, this, &DownloadManager::downloadFinished);
        connect(currentReply, &QNetworkReply::readyRead, this, &DownloadManager::downloadReadyRead);
    }
};

#endif // DOWNLOADMANAGER_HPP
