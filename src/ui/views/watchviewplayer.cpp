#include "watchviewplayer.h"
#include "innertube.h"

#ifdef QTTUBE_USE_MPV
#include "http.h"
#include "lib/media/mpv/mediampv.h"
#include "settingsstore.h"
#include <QMessageBox>
#include <QRandomGenerator>
#endif

WatchViewPlayer::WatchViewPlayer(QWidget* watchView, const QSize& maxSize) : QObject(watchView)
{
#ifdef QTTUBE_USE_MPV
    media = new MediaMPV(watchView);
    media->init();
    media->setVolume(SettingsStore::instance()->preferredVolume);

    connect(media, &Media::error, this, [](const QString& message) { qWarning() << "Media error:" << message; });
    connect(media, &Media::stateChanged, this, &WatchViewPlayer::mediaStateChanged);
    connect(media, &Media::volumeChanged, this, &WatchViewPlayer::volumeChanged);
#else
    wePlayer = new WebEnginePlayer(watchView);
    wePlayer->setAuthStore(InnerTube::instance().authStore());
    wePlayer->setContext(InnerTube::instance().context());
#endif

    calcAndSetSize(maxSize);
}

void WatchViewPlayer::calcAndSetSize(const QSize& maxSize)
{
    int playerWidth = maxSize.width();
    int playerHeight = playerWidth * 9/16;

    if (playerHeight > maxSize.height() - 150)
    {
        playerHeight = maxSize.height() - 150;
        playerWidth = playerHeight * 16/9;
    }

    QSize sz(playerWidth, playerHeight);
    m_size = sz;
    widget()->setFixedSize(sz);
}

void WatchViewPlayer::play(const QString& videoId, int progress)
{
#ifdef QTTUBE_USE_MPV
    media->play("https://www.youtube.com/watch?v=" + videoId);
    media->seek(progress);
#else
    wePlayer->play(videoId, progress);
#endif
}

void WatchViewPlayer::seek(int progress)
{
#ifdef QTTUBE_USE_MPV
    media->seek(progress);
#else
    wePlayer->seek(progress);
#endif
}

void WatchViewPlayer::startTracking(const InnertubeEndpoints::PlayerResponse& playerResp)
{
#ifdef QTTUBE_USE_MPV
    if (SettingsStore::instance()->playbackTracking)
        reportPlayback(playerResp);

    if (SettingsStore::instance()->watchtimeTracking)
    {
        watchtimeTimer = new QTimer(this);
        watchtimeTimer->setInterval(5000);
        connect(watchtimeTimer, &QTimer::timeout, this, std::bind(&WatchViewPlayer::reportWatchtime, this, playerResp, media->position()));
        watchtimeTimer->start();
    }
#else
    wePlayer->setPlayerResponse(playerResp);
#endif
}

void WatchViewPlayer::stopTracking()
{
#ifdef QTTUBE_USE_MPV
    if (watchtimeTimer)
        watchtimeTimer->deleteLater();
#endif
}

QWidget* WatchViewPlayer::widget()
{
#ifdef QTTUBE_USE_MPV
    return media->videoWidget();
#else
    return wePlayer;
#endif
}

#ifdef QTTUBE_USE_MPV
QString WatchViewPlayer::getCpn()
{
    QString out;
    constexpr std::string_view chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    for (int i = 0; i < 16; i++)
        out += chars[QRandomGenerator::global()->bounded((int)chars.size())];
    return out;
}

void WatchViewPlayer::mediaStateChanged(Media::State state)
{
    if (state == Media::ErrorState)
        QMessageBox::critical(nullptr, "Media error", media->errorString());
}

void WatchViewPlayer::reportPlayback(const InnertubeEndpoints::PlayerResponse& playerResp)
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
        { "rt", QString::number(QRandomGenerator::global()->bounded(191) + 10) },
        { "euri", "" },
        { "lact", QString::number(QRandomGenerator::global()->bounded(7001) + 1000) },
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

void WatchViewPlayer::reportWatchtime(const InnertubeEndpoints::PlayerResponse& playerResp, long long position)
{
    InnertubeClient itc = InnerTube::instance().context()->client;

    QUrlQuery watchtimeQuery(QUrl(playerResp.playbackTracking.videostatsWatchtimeUrl));
    QUrl outWatchtimeUrl("https://www.youtube.com/api/stats/watchtime");
    QUrlQuery outWatchtimeQuery;
    QString rt = QString::number(QRandomGenerator::global()->bounded(191) + 10);
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
        { "lact", QString::number(QRandomGenerator::global()->bounded(7001) + 1000) },
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

void WatchViewPlayer::volumeChanged(double volume)
{
    Q_UNUSED(volume);
    if (media->volumeMuted())
        media->setVolumeMuted(false);
}
#endif
