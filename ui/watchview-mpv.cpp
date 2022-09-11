#ifdef USEMPV
#include "mainwindow.h"
#include "media/mpv/mediampv.h"
#include "settingsstore.hpp"
#include "watchview-mpv.h"
#include <QVBoxLayout>

WatchView* WatchView::instance()
{
    static WatchView* wv = new WatchView;
    return wv;
}

WatchView::WatchView(QWidget* parent) : QWidget(parent)
{
    grid = new QGridLayout(this);
    setLayout(grid);
}

void WatchView::initialize(QStackedWidget* stackedWidget)
{
    this->stackedWidget = stackedWidget;

    QVBoxLayout* info = new QVBoxLayout(this);
    info->setContentsMargins(0, 0, 0, 0);

    recommendations = new QListWidget(this);

    for (int i = 0; i < 10; ++i)
    {
        QAction* seekAction = new QAction;
        seekAction->setShortcut(Qt::Key_0 + i);
        seekAction->setAutoRepeat(false);
        connect(seekAction, &QAction::triggered, this, [i, this] { media->seek((media->duration() * (i * 10)) / 100); });
        addAction(seekAction);
    }

    QAction* backAction = new QAction("Back 10 seconds");
    backAction->setShortcuts(QList<QKeySequence>() << QKeySequence(Qt::Key_Left) << QKeySequence(Qt::Key_MediaPrevious));
    backAction->setAutoRepeat(false);
    connect(backAction, &QAction::triggered, this, [this] { media->relativeSeek(-10000); });
    addAction(backAction);

    QAction* forwardAction = new QAction("Forward 10 seconds");
    forwardAction->setShortcuts(QList<QKeySequence>() << QKeySequence(Qt::Key_Right) << QKeySequence(Qt::Key_MediaNext));
    forwardAction->setAutoRepeat(false);
    connect(forwardAction, &QAction::triggered, this, [this] { media->relativeSeek(10000); });
    addAction(forwardAction);

    QAction* pauseAction = new QAction("Toggle pause");
    pauseAction->setShortcuts(QList<QKeySequence>() << QKeySequence(Qt::Key_Space) << QKeySequence(Qt::Key_K) << QKeySequence(Qt::Key_MediaPlay) << QKeySequence(Qt::Key_MediaTogglePlayPause) << QKeySequence(Qt::Key_MediaPause));

    media = new MediaMPV;
    media->init();
    media->setVolume(SettingsStore::instance().preferredVolume);

    connect(media, &Media::error, this, [this](const QString& message) { QMessageBox::warning(this, "Media error", message); });
    connect(media, &Media::stateChanged, this, &WatchView::mediaStateChanged);
    connect(media, &Media::volumeChanged, this, &WatchView::volumeChanged);
    // TODO: add volume icon, slider, etc.
}

void WatchView::loadVideo(const InnertubeEndpoints::Player& player)
{
    stackedWidget->setCurrentIndex(1);
    MainWindow::instance()->setWindowTitle(player.videoDetails.title + " - youtube-qt");
    if (player.videoDetails.isLive || player.videoDetails.isLiveContent)
    {
        media->play(player.streamingData.hlsManifestUrl);
    }
    else
    {
        QList<InnertubeObjects::StreamingFormat> audioFormats, videoFormats;
        for (const InnertubeObjects::StreamingFormat& f : player.streamingData.adaptiveFormats)
        {
            if (f.mimeType.startsWith("audio/"))
                audioFormats.append(f);
            else if (f.mimeType.startsWith("video/"))
                videoFormats.append(f);
        }

        InnertubeObjects::StreamingFormat bestAudio = *std::ranges::max_element(audioFormats, [](const auto& a, const auto& b) { return a.bitrate < b.bitrate; });
        InnertubeObjects::StreamingFormat bestVideo = *std::ranges::max_element(videoFormats, [](const auto& a, const auto& b) { return a.height < b.height; });
        media->playSeparateAudioAndVideo(bestVideo.url, bestAudio.url);
    }
}

void WatchView::mediaStateChanged(Media::State state)
{
    if (state == Media::ErrorState)
        QMessageBox::critical(this, "Media error", media->errorString());
}

void WatchView::volumeChanged(double volume)
{
    Q_UNUSED(volume);
    if (media->volumeMuted())
        media->setVolumeMuted(false);
}
#endif // USEMPV
