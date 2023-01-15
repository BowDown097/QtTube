#include "watchview.h"
#include "channelview.h"
#include "http.h"
#include "innertube.h"
#include "settingsstore.h"
#include "ui/forms/mainwindow.h"
#include "ui/uiutilities.h"
#include <QApplication>
#include <QMessageBox>
#include <QResizeEvent>

#ifdef USEMPV
#include "lib/media/mpv/mediampv.h"
#endif

#if defined(Q_OS_UNIX) && !defined(__APPLE__) && !defined(__MACH__)
#include <X11/extensions/scrnsaver.h>
#elif defined (Q_OS_WIN)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

WatchView* WatchView::instance()
{
    if (!m_watchView)
        m_watchView = new WatchView;
    return m_watchView;
}

void WatchView::goBack()
{
#ifdef USEMPV
    media->stop();
    media->clearQueue();
    watchtimeTimer->deleteLater();

    for (QAction* action : actions())
        removeAction(action);
#endif
    MainWindow::topbar()->alwaysShow = true;
    MainWindow::centralWidget()->setCurrentIndex(0);
    disconnect(MainWindow::topbar()->logo, &TubeLabel::clicked, this, &WatchView::goBack);

    UIUtilities::clearLayout(pageLayout);
    pageLayout->deleteLater();
    toggleIdleSleep(false);
}

void WatchView::loadVideo(const QString& videoId, int progress)
{
    auto nextResp = InnerTube::instance().get<InnertubeEndpoints::Next>(videoId).response;
    auto playerResp = InnerTube::instance().get<InnertubeEndpoints::Player>(videoId).response;

    MainWindow::centralWidget()->setCurrentIndex(1);

    pageLayout = new QVBoxLayout(this);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->setSpacing(5);

    QSize playerSize = calcPlayerSize();
#ifdef USEMPV
    media = new MediaMPV;
    media->init();
    media->setVolume(SettingsStore::instance().preferredVolume);
    media->videoWidget()->setFixedSize(playerSize);
    pageLayout->addWidget(media->videoWidget());

    connect(media, &Media::error, this, [this](const QString& message) { QMessageBox::warning(this, "Media error", message); });
    connect(media, &Media::stateChanged, this, &WatchView::mediaStateChanged);
    connect(media, &Media::volumeChanged, this, &WatchView::volumeChanged);
#else
    wePlayer = new WebEnginePlayer(this);
    wePlayer->setAuthStore(InnerTube::instance().authStore());
    wePlayer->setContext(InnerTube::instance().context());
    pageLayout->addWidget(wePlayer);
#endif

    titleLabel = new TubeLabel(playerResp.videoDetails.title, this);
    titleLabel->setFixedWidth(playerSize.width());
    titleLabel->setFont(QFont(qApp->font().toString(), qApp->font().pointSize() + 4));
    titleLabel->setWordWrap(true);
    pageLayout->addWidget(titleLabel);

    primaryInfoHbox = new QHBoxLayout;
    primaryInfoHbox->setContentsMargins(0, 0, 0, 0);

    channelIcon = new TubeLabel(this);
    channelIcon->setClickable(true, false);
    channelIcon->setMaximumSize(55, 48);
    channelIcon->setMinimumSize(55, 48);
    primaryInfoHbox->addWidget(channelIcon);

    primaryInfoVbox = new QVBoxLayout;

    channelName = new TubeLabel(this);
    channelName->setClickable(true, true);
    channelName->setText(nextResp.secondaryInfo.channelName.text);
    primaryInfoVbox->addWidget(channelName);
    connect(channelName, &TubeLabel::clicked, this, [this, nextResp] {
        disconnect(MainWindow::topbar()->logo, &TubeLabel::clicked, this, &WatchView::goBack);
        toggleIdleSleep(false);
        navigateChannel(nextResp.secondaryInfo.subscribeButton.channelId);
        UIUtilities::clearLayout(pageLayout);
        pageLayout->deleteLater();
    });

    subscribeHbox = new QHBoxLayout(this);
    subscribeHbox->setContentsMargins(0, 0, 0, 0);
    subscribeHbox->setSpacing(0);

    subscribeWidget = new SubscribeWidget(nextResp.secondaryInfo.subscribeButton, this);
    subscribeHbox->addWidget(subscribeWidget);

    subscribersLabel = new TubeLabel(this);
    subscribersLabel->setStyleSheet(R"(
    border: 1px solid #333;
    font-size: 11px;
    line-height: 24px;
    padding: 0 6px 0 4.5px;
    border-radius: 2px;
    text-align: center;
    )");
    setSubscriberCount(nextResp.secondaryInfo);
    subscribeHbox->addWidget(subscribersLabel);

    primaryInfoVbox->addLayout(subscribeHbox);
    primaryInfoHbox->addLayout(primaryInfoVbox);
    primaryInfoHbox->addStretch();

    primaryInfoWrapper = new QWidget(this);
    primaryInfoWrapper->setFixedWidth(playerSize.width());
    primaryInfoWrapper->setLayout(primaryInfoHbox);
    pageLayout->addWidget(primaryInfoWrapper);

#ifndef USEMPV
    wePlayer->play(playerResp.videoDetails.videoId, progress);
    wePlayer->setFixedSize(playerSize);
    wePlayer->setPlayerResponse(playerResp);
#endif

    pageLayout->addStretch(); // disable the layout from stretching on resize

    if (!nextResp.secondaryInfo.channelIcons.isEmpty())
    {
        QList<InnertubeObjects::GenericThumbnail> channelIcons = nextResp.secondaryInfo.channelIcons;
        auto bestThumb = *std::find_if(channelIcons.cbegin(), channelIcons.cend(), [](const InnertubeObjects::GenericThumbnail& t)
        {
            return t.width >= 48;
        });
        HttpReply* reply = Http::instance().get(bestThumb.url);
        connect(reply, &HttpReply::finished, this, &WatchView::setChannelIcon);
    }

    setWindowTitle(playerResp.videoDetails.title + " - QtTube");
    toggleIdleSleep(true);

    MainWindow::topbar()->setVisible(false);
    MainWindow::topbar()->alwaysShow = false;
    connect(MainWindow::topbar()->logo, &TubeLabel::clicked, this, &WatchView::goBack);

#ifdef USEMPV
    media->play("https://www.youtube.com/watch?v=" + playerResp.videoDetails.videoId);
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
#endif
}

void WatchView::resizeEvent(QResizeEvent* event)
{
    if (!primaryInfoWrapper || !event->oldSize().isValid()) return;

    QSize playerSize = calcPlayerSize();
    primaryInfoWrapper->setFixedWidth(playerSize.width());
    titleLabel->setFixedWidth(playerSize.width());

#ifdef USEMPV
    media->videoWidget()->setFixedSize(playerSize);
#else
    wePlayer->setFixedSize(playerSize);
#endif
}

QSize WatchView::calcPlayerSize()
{
    int playerWidth = width();
    int playerHeight = playerWidth * 9/16;

    if (playerHeight > height() - 125)
    {
        playerHeight = height() - 125;
        playerWidth = playerHeight * 16/9;
    }

    return QSize(playerWidth, playerHeight);
}

void WatchView::navigateChannel(const QString& channelId)
{
    try
    {
        ChannelView::instance()->loadChannel(channelId);
    }
    catch (const InnertubeException& ie)
    {
        QMessageBox::critical(nullptr, "Failed to load channel", ie.message());
    }
}

void WatchView::setChannelIcon(const HttpReply& reply)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    channelIcon->setPixmap(pixmap.scaled(48, 48, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void WatchView::setSubscriberCount(const InnertubeObjects::VideoSecondaryInfo& secondaryInfo)
{
    QString subscriberCountText = secondaryInfo.subscriberCountText.text;
    if (!SettingsStore::instance().fullSubs)
    {
        subscribersLabel->setText(subscriberCountText.first(subscriberCountText.lastIndexOf(" ")));
        subscribersLabel->adjustSize();
        return;
    }

    Http http;
    http.setReadTimeout(2000);
    http.setMaxRetries(5);

    HttpReply* reply = http.get(QUrl("https://api.socialcounts.org/youtube-live-subscriber-count/" + secondaryInfo.subscribeButton.channelId));
    connect(reply, &HttpReply::error, this, [this, subscriberCountText] {
        subscribersLabel->setText(subscriberCountText.first(subscriberCountText.lastIndexOf(" ")));
        subscribersLabel->adjustSize();
    });
    connect(reply, &HttpReply::finished, this, [this](const HttpReply& reply) {
        int subs = QJsonDocument::fromJson(reply.body())["est_sub"].toInt();
        subscribersLabel->setText(QLocale::system().toString(subs));
        subscribersLabel->adjustSize();
    });
}

void WatchView::toggleIdleSleep(bool toggle)
{
#if defined(Q_OS_UNIX) && !defined(__APPLE__) && !defined(__MACH__)
    Display* display = qApp->nativeInterface<QNativeInterface::QX11Application>()->display();
    if (!display)
    {
        qDebug() << "Failed to toggle idle sleep: Failed to get X11 display";
        return;
    }

    int event, error, major, minor;
    if (XScreenSaverQueryExtension(display, &event, &error) != 1)
    {
        qDebug() << "Failed to toggle idle sleep: XScreenSaverQueryExtension failed";
        return;
    }
    if (XScreenSaverQueryVersion(display, &major, &minor) != 1 || major < 1 || (major == 1 && minor < 1))
    {
        qDebug() << "Failed to toggle idle sleep: XScreenSaverQueryVersion failed or definitely returned the wrong version";
        return;
    }

    XScreenSaverSuspend(display, toggle);
#elif defined(Q_OS_WIN)
    if (SetThreadExecutionState(toggle ? ES_DISPLAY_REQUIRED | ES_CONTINUOUS | ES_SYSTEM_REQUIRED : ES_CONTINUOUS) == NULL)
        qDebug() << "Failed to toggle idle sleep: SetThreadExecutionState failed";
#elif defined(Q_OS_MACOS)
    if (!toggle && sleepAssert)
    {
        IOPMAssertionRelease(sleepAssert);
        return;
    }

    CFStringRef reason = CFSTR("QtTube video playing");
    IOReturn success = IOPMAssertionCreateWithName(kIOPMAssertionTypeNoDisplaySleep, kIOPMAssertionLevelOn, reason, &sleepAssert);
    if (success != kIOReturnSuccess)
        qDebug() << "Failed to toggle idle sleep: Creating IOPM assertion failed";
#else
    qDebug() << "Failed to toggle idle sleep: Unsupported OS";
#endif
}


#ifdef USEMPV // MPV backend exclusive methods
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
    InnertubeClient itc = InnerTube::instance().context()->client;

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
    InnertubeClient itc = InnerTube::instance().context()->client;

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
#endif // MPV backend exclusive methods
