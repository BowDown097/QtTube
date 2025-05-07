#pragma once
#include <QDir>
#include <QList>
#include <QQueue>
#include <QUrl>
#include <QWidget>
#include <mutex>

class DownloadEntity;
class QHBoxLayout;
class QLabel;
class QNetworkAccessManager;
class QPushButton;
class QThreadPool;
class QVBoxLayout;

class DownloadManager : public QWidget
{
    Q_OBJECT
public:
    static DownloadManager* instance();
    void append(const QString& videoId);

    const QDir& directory() const { return m_directory; }
    void setDirectory(const QDir& directory) { m_directory = directory; }
private:
    static inline DownloadManager* m_instance;
    static inline std::once_flag m_onceFlag;

    QPushButton* m_cancelButton;
    QDir m_directory;
    QList<DownloadEntity*> m_downloads;
    QHBoxLayout* m_footerLayout;
    QVBoxLayout* m_layout;
    QNetworkAccessManager* m_manager;
    QVBoxLayout* m_progressLayout;
    QQueue<QUrl> m_queue;
    QLabel* m_queueCountLabel;

    explicit DownloadManager(QWidget* parent = nullptr);
    bool isDuplicate(const QUrl& url);
    void removeEntity(DownloadEntity* entity, bool cleanUp);
    void setUpEntity(DownloadEntity* entity);
    void startDownload(const QUrl& url);
    void tryStartQueuedEntities();
private slots:
    void cancel();
    void downloadFinished(bool cancelled);
    void downloadRequestSent();
};
