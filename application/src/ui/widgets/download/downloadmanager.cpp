#include "downloadmanager.h"
#include "downloadentity.h"
#include "qttubeapplication.h"
#include <QBoxLayout>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QPushButton>
#include <QStandardPaths>

constexpr int MaxDownloads = 5;

DownloadManager* DownloadManager::instance()
{
    std::call_once(m_onceFlag, [] { m_instance = new DownloadManager; });
    return m_instance;
}

DownloadManager::DownloadManager(QWidget* parent)
    : QWidget(parent),
      m_cancelButton(new QPushButton("Cancel", this)),
      m_footerLayout(new QHBoxLayout),
      m_layout(new QVBoxLayout(this)),
      m_manager(new QNetworkAccessManager(this)),
      m_progressLayout(new QVBoxLayout),
      m_queueCountLabel(new QLabel(this))
{
    if (const QString& downloadPath = qtTubeApp->settings().downloadPath; !downloadPath.isEmpty())
        m_directory.setPath(downloadPath);
    else
        m_directory.setPath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/downloads");

    setWindowTitle("Download Manager");
    m_cancelButton->setFixedWidth(70);

    m_footerLayout->addWidget(m_cancelButton, 0, Qt::AlignLeft);
    m_footerLayout->addWidget(m_queueCountLabel, 0, Qt::AlignRight);

    m_layout->addLayout(m_progressLayout);
    m_layout->addLayout(m_footerLayout);

    connect(m_cancelButton, &QPushButton::clicked, this, &DownloadManager::cancel);
    resize(800, sizeHint().height());
}

void DownloadManager::append(const QString& videoId)
{
    QUrl url("https://youtu.be/" + videoId);
    if (isDuplicate(url))
        return;

    m_queue.append(url);
    tryStartQueuedEntities();
}

void DownloadManager::cancel()
{
    m_queue.clear();
    m_queueCountLabel->clear();

    for (DownloadEntity* entity : std::as_const(m_downloads))
    {
        removeEntity(entity, true);
        entity->deleteLater();
        m_downloads.removeOne(entity);
    }

    hide();
}

void DownloadManager::downloadFinished(bool cancelled)
{
    if (DownloadEntity* entity = qobject_cast<DownloadEntity*>(sender()))
        removeEntity(entity, cancelled);

    tryStartQueuedEntities();
    if (m_downloads.isEmpty() && m_queue.isEmpty())
        hide();
    else
        resize(width(), sizeHint().height());
}

void DownloadManager::downloadRequestSent()
{
    if (DownloadEntity* entity = qobject_cast<DownloadEntity*>(sender()))
    {
        show();
        entity->show();
        m_progressLayout->addWidget(entity);
        resize(width(), sizeHint().height());
    }
}

bool DownloadManager::isDuplicate(const QUrl& url)
{
    if (std::ranges::find(m_downloads, url, &DownloadEntity::url) != m_downloads.end())
        return true;
    if (m_queue.contains(url))
        return true;
    return false;
}

void DownloadManager::removeEntity(DownloadEntity* entity, bool cleanUp)
{
    if (cleanUp)
        entity->cleanUp();
    entity->deleteLater();
    m_downloads.removeOne(entity);
}

void DownloadManager::setUpEntity(DownloadEntity* entity)
{
    entity->hide();
    entity->setFixedHeight(50);
    connect(entity, &DownloadEntity::finished, this, &DownloadManager::downloadFinished);
    connect(entity, &DownloadEntity::requestSent, this, &DownloadManager::downloadRequestSent);
    m_downloads.append(entity);
}

void DownloadManager::startDownload(const QUrl& url)
{
    DownloadEntity* entity = new DownloadEntity(m_directory, this);
    setUpEntity(entity);
    entity->startDownload(url);
}

void DownloadManager::tryStartQueuedEntities()
{
    while (!m_queue.isEmpty() && m_downloads.size() <= MaxDownloads)
        startDownload(m_queue.dequeue());
    m_queueCountLabel->setText(!m_queue.isEmpty() ? QStringLiteral("In queue: %1").arg(m_queue.size()) : QString());
}
