#ifdef USEMPV
#include "../settingsstore.h"
#include "lib/media/mpv/mediampv.h"
#include "mainwindow.h"
#include "watchview-mpv.h"
#include "watchview-shared.hpp"
#include <QVBoxLayout>

WatchView* WatchView::instance()
{
    static WatchView* wv = new WatchView;
    return wv;
}

void WatchView::goBack()
{
    MainWindow::instance()->topbar->alwaysShow = true;
    disconnect(MainWindow::instance()->topbar->logo, &ClickableLabel::clicked, this, &WatchView::goBack);
    UIUtilities::clearLayout(pageLayout);
    pageLayout->deleteLater();
    stackedWidget->setCurrentIndex(0);
    WatchViewShared::toggleIdleSleep(false);
}

void WatchView::initialize(const InnertubeClient& client, QStackedWidget* stackedWidget)
{
    this->itc = client;
    this->stackedWidget = stackedWidget;

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
    // TODO: add volume icon, slider, etc.
}

void WatchView::loadVideo(const InnertubeEndpoints::NextResponse& nextResp, const InnertubeEndpoints::PlayerResponse& playerResp, int progress)
{
    stackedWidget->setCurrentIndex(1);

    pageLayout = new QVBoxLayout(this);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->setSpacing(5);

    media = new MediaMPV;
    media->init();
    media->setVolume(SettingsStore::instance().preferredVolume);
    media->videoWidget()->setFixedSize(WatchViewShared::calcPlayerSize(width(), MainWindow::instance()->height()));
    pageLayout->addWidget(media->videoWidget());

    connect(media, &Media::error, this, [this](const QString& message) { QMessageBox::warning(this, "Media error", message); });
    connect(media, &Media::stateChanged, this, &WatchView::mediaStateChanged);
    connect(media, &Media::volumeChanged, this, &WatchView::volumeChanged);

    titleLabel = new QLabel(this);
    titleLabel->setFont(QFont(QApplication::font().toString(), QApplication::font().pointSize() + 4));
    titleLabel->setText(playerResp.videoDetails.title);
    titleLabel->setWordWrap(true);
    pageLayout->addWidget(titleLabel);

    primaryInfoHbox = new QHBoxLayout;
    primaryInfoHbox->setContentsMargins(0, 0, 0, 0);
    primaryInfoHbox->setSizeConstraint(QBoxLayout::SizeConstraint::SetFixedSize);

    channelIcon = new ClickableLabel(false, this);
    channelIcon->setMaximumSize(55, 48);
    channelIcon->setMinimumSize(55, 48);
    primaryInfoHbox->addWidget(channelIcon);

    primaryInfoVbox = new QVBoxLayout;

    channelName = new ClickableLabel(true, this);
    channelName->setText(nextResp.secondaryInfo.channelName.text);
    primaryInfoVbox->addWidget(channelName);

    subscribersLabel = new QLabel(this);
    WatchViewShared::setSubscriberCount(nextResp.secondaryInfo, subscribersLabel);
    primaryInfoVbox->addWidget(subscribersLabel);

    primaryInfoHbox->addLayout(primaryInfoVbox);
    pageLayout->addLayout(primaryInfoHbox);

    pageLayout->addStretch(); // disable the layout from stretching on resize

    if (!nextResp.secondaryInfo.channelIcons.isEmpty())
    {
        auto bestThumb = *std::ranges::find_if(nextResp.secondaryInfo.channelIcons, [](const auto& t) { return t.width >= 48; });
        HttpReply* reply = Http::instance().get(bestThumb.url);
        connect(reply, &HttpReply::finished, this, [this](const HttpReply& reply) { WatchViewShared::setChannelIcon(reply, channelIcon); });
    }

    MainWindow::instance()->setWindowTitle(playerResp.videoDetails.title + " - QtTube");
    WatchViewShared::toggleIdleSleep(true);

    MainWindow::instance()->topbar->setVisible(false);
    MainWindow::instance()->topbar->alwaysShow = false;
    connect(MainWindow::instance()->topbar->logo, &ClickableLabel::clicked, this, &WatchView::goBack);
    if (playerResp.videoDetails.isLive || playerResp.videoDetails.isLiveContent)
    {
        media->play(playerResp.streamingData.hlsManifestUrl);
    }
    else
    {
        QList<InnertubeObjects::StreamingFormat> audioFormats, videoFormats;
        for (const InnertubeObjects::StreamingFormat& f : playerResp.streamingData.adaptiveFormats)
        {
            if (f.mimeType.startsWith("audio/"))
                audioFormats.append(f);
            else if (f.mimeType.startsWith("video/"))
                videoFormats.append(f);
        }

        auto bestAudio = *std::ranges::max_element(audioFormats, [](const auto& a, const auto& b) { return a.bitrate < b.bitrate; });
        auto bestVideo = *std::ranges::max_element(videoFormats, [](const auto& a, const auto& b) { return a.height < b.height; });
        media->playSeparateAudioAndVideo(bestVideo.url, bestAudio.url);
    }

    media->seek(progress);

    if (SettingsStore::instance().playbackTracking)
        reportPlayback(playerResp);

    if (SettingsStore::instance().watchtimeTracking)
    {
        watchtimeTimer = new QTimer;
        watchtimeTimer->setInterval(5000);
        connect(watchtimeTimer, &QTimer::timeout, this, [this, playerResp] { reportWatchtime(playerResp, media->position()); });
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

void WatchView::reportPlayback(const InnertubeEndpoints::PlayerResponse& playerResp)
{
    QUrlQuery playbackQuery(QUrl(playerResp.playbackTracking.videostatsPlaybackUrl));

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
        { "cbr", itc.browserName },
        { "cbrver", itc.browserVersion },
        { "c", itc.clientName },
        { "cver", itc.clientVersion },
        { "cplayer", "UNIPLAYER" },
        { "cos", itc.osName },
        { "cosver", itc.osVersion },
        { "cplatform", itc.platform },
        { "hl", itc.hl + "_" + itc.gl },
        { "cr", itc.gl },
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

void WatchView::reportWatchtime(const InnertubeEndpoints::PlayerResponse& playerResp, long long position)
{
    QUrlQuery watchtimeQuery(QUrl(playerResp.playbackTracking.videostatsWatchtimeUrl));
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
        { "cbr", itc.browserName },
        { "cbrver", itc.browserVersion },
        { "c", itc.clientName },
        { "cver", itc.clientVersion },
        { "cplayer", "UNIPLAYER" },
        { "cos", itc.osName },
        { "cosver", itc.osVersion },
        { "cplatform", itc.platform },
        { "hl", itc.hl + "_" + itc.gl },
        { "cr", itc.gl },
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
