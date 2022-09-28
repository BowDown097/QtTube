#ifdef USEMPV
#include "http.h"
#include "lib/media/mpv/mediampv.h"
#include "mainwindow.h"
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

void WatchView::initialize(const InnertubeClient& client, QStackedWidget* stackedWidget)
{
    this->itc = client;
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

    if (SettingsStore::instance().playbackTracking)
        reportPlayback(itc, player);

    if (SettingsStore::instance().watchtimeTracking)
    {
        watchtimeTimer = new QTimer;
        watchtimeTimer->setInterval(5000);
        connect(watchtimeTimer, &QTimer::timeout, this, [=]() { reportWatchtime(itc, player, media->position()); });
        watchtimeTimer->start();
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

QString WatchView::getCpn()
{
    QString out;
    constexpr std::string_view chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    for (int i = 0; i < 16; i++)
        out += chars[rand() % chars.size()];
    return out;
}

void WatchView::reportPlayback(const InnertubeClient& client, const InnertubeEndpoints::Player& player)
{
    QUrlQuery playbackQuery(QUrl(player.playbackTracking.videostatsPlaybackUrl));

    QUrl outPlaybackUrl("https://www.youtube.com/api/stats/playback");
    QUrlQuery outPlaybackQuery;
    QList<QPair<QString, QString>> map =
    {
        { "ns", "yt" },
        { "el", "detailpage" },
        { "cpn", getCpn() },
        { "ver", "2" },
        { "fmt", "243" },
        { "fs", "0" },
        { "rt", QString::number((rand() % 191) + 10) },
        { "euri", "" },
        { "lact", QString::number((rand() % 7001) + 1000) },
        { "cl", playbackQuery.queryItemValue("cl") },
        { "mos", "0" },
        { "volume", "100" },
        { "cbr", client.browserName },
        { "cbrver", client.browserVersion },
        { "c", client.clientName },
        { "cver", client.clientVersion },
        { "cplayer", "UNIPLAYER" },
        { "cos", client.osName },
        { "cosver", client.osVersion },
        { "cplatform", client.platform },
        { "hl", client.hl + "_" + client.gl },
        { "cr", client.gl },
        { "uga", playbackQuery.queryItemValue("uga") },
        { "len", playbackQuery.queryItemValue("len") },
        { "fexp", playbackQuery.queryItemValue("fexp") },
        { "rtn", "4" },
        { "afmt", "251" },
        { "muted", "0" },
        { "docid", playbackQuery.queryItemValue("docid") },
        { "ei", playbackQuery.queryItemValue("ei") },
        { "plid", playbackQuery.queryItemValue("plid") },
        { "sdetail", playbackQuery.queryItemValue("sdetail") },
        { "of", playbackQuery.queryItemValue("of") },
        { "vm", playbackQuery.queryItemValue("vm") }
    };
    outPlaybackQuery.setQueryItems(map);
    outPlaybackUrl.setQuery(outPlaybackQuery);
    Http::instance().get(outPlaybackUrl);
}

void WatchView::reportWatchtime(const InnertubeClient& client, const InnertubeEndpoints::Player& player, long long position)
{
    QUrlQuery watchtimeQuery(QUrl(player.playbackTracking.videostatsWatchtimeUrl));
    QUrl outWatchtimeUrl("https://www.youtube.com/api/stats/watchtime");
    QUrlQuery outWatchtimeQuery;
    QString rt = QString::number((rand() % 191) + 10);
    QString posStr = QString::number(position);

    QList<QPair<QString, QString>> map =
    {
        { "ns", "yt" },
        { "el", "detailpage" },
        { "cpn", getCpn() },
        { "ver", "2" },
        { "fmt", "243" },
        { "fs", "0" },
        { "rt", rt },
        { "euri", "" },
        { "lact", QString::number((rand() % 7001) + 1000) },
        { "cl", watchtimeQuery.queryItemValue("cl") },
        { "state", "playing" },
        { "volume", "100" },
        { "subscribed", watchtimeQuery.queryItemValue("subscribed") },
        { "cbr", client.browserName },
        { "cbrver", client.browserVersion },
        { "c", client.clientName },
        { "cver", client.clientVersion },
        { "cplayer", "UNIPLAYER" },
        { "cos", client.osName },
        { "cosver", client.osVersion },
        { "cplatform", client.platform },
        { "hl", client.hl + "_" + client.gl },
        { "cr", client.gl },
        { "uga", watchtimeQuery.queryItemValue("uga") },
        { "len", watchtimeQuery.queryItemValue("len") },
        { "afmt", "251" },
        { "idpj", "-1" },
        { "ldpj", "-10" },
        { "rti", rt },
        { "st", posStr },
        { "et", posStr },
        { "muted", "0" },
        { "docid", watchtimeQuery.queryItemValue("docid") },
        { "ei", watchtimeQuery.queryItemValue("ei") },
        { "plid", watchtimeQuery.queryItemValue("plid") },
        { "sdetail", watchtimeQuery.queryItemValue("sdetail") },
        { "of", watchtimeQuery.queryItemValue("of") },
        { "vm", watchtimeQuery.queryItemValue("vm") }
    };
    outWatchtimeQuery.setQueryItems(map);
    outWatchtimeUrl.setQuery(outWatchtimeQuery);
    Http::instance().get(outWatchtimeUrl);
}
#endif // USEMPV
