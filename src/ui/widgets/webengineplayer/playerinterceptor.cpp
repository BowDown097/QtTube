#include "playerinterceptor.h"
#include "qttubeapplication.h"
#include "utils/statsutils.h"
#include <QUrlQuery>

void PlayerInterceptor::interceptRequest(QWebEngineUrlRequestInfo& info)
{
    if (!m_authStore || !m_context)
    {
        qDebug() << "Auth store or InnerTube context is null - using YouTube's default tracking.";
        return;
    }

    // block for privacy, should not have any impact on the operation of the player or program
    const QUrl url = info.requestUrl();
    if (url.host().contains("doubleclick.net") || url.host() == "jnn-pa.googleapis.com" ||
        url.path() == "/youtubei/v1/log_event" || url.path() == "/api/stats/qoe" ||
        url.path() == "/ptracking" || url.toString().contains("play.google.com/log"))
    {
        info.block(true);
    }

    // modify based on settings
    if (url.path() == "/api/stats/watchtime")
    {
        info.block(true);
        if (!qtTubeApp->settings().watchtimeTracking) return;

        QUrlQuery watchtimeQuery(url);
        QUrlQuery playerWatchtimeQuery(QUrl(m_playerResponse.playbackTracking.videostatsWatchtimeUrl));

        QUrl outWatchtimeUrl("https://www.youtube.com/api/stats/watchtime");
        QUrlQuery outWatchtimeQuery;
        QList<QPair<QString, QString>> map =
        {
            { "ns", "yt" },
            { "el", "detailpage" },
            { "cpn", watchtimeQuery.queryItemValue("cpn") },
            { "ver", "2" },
            { "fmt", watchtimeQuery.queryItemValue("fmt") },
            { "fs", "0" },
            { "rt", watchtimeQuery.queryItemValue("rt") },
            { "euri", "" },
            { "lact", watchtimeQuery.queryItemValue("lact") },
            { "cl", playerWatchtimeQuery.queryItemValue("cl") },
            { "state", watchtimeQuery.queryItemValue("state") },
            { "volume", watchtimeQuery.queryItemValue("volume") },
            { "subscribed", playerWatchtimeQuery.queryItemValue("subscribed") },
            { "cbr", m_context->client.browserName },
            { "cbrver", m_context->client.browserVersion },
            { "c", QString::number(static_cast<int>(m_context->client.clientType)) },
            { "cver", m_context->client.clientVersion },
            { "cplayer", "UNIPLAYER" },
            { "cos", m_context->client.osName },
            { "cosver", m_context->client.osVersion },
            { "cplatform", m_context->client.platform },
            { "hl", watchtimeQuery.queryItemValue("hl") },
            { "cr", watchtimeQuery.queryItemValue("cr") },
            { "uga", playerWatchtimeQuery.queryItemValue("uga") },
            { "len", watchtimeQuery.queryItemValue("len") },
            { "afmt", watchtimeQuery.queryItemValue("afmt") },
            { "idpj", watchtimeQuery.queryItemValue("idpj") },
            { "ldpj", watchtimeQuery.queryItemValue("ldpj") },
            { "rti", watchtimeQuery.queryItemValue("rti") },
            { "st", watchtimeQuery.queryItemValue("st") },
            { "et", watchtimeQuery.queryItemValue("et") },
            { "muted", watchtimeQuery.queryItemValue("muted") },
            { "docid", watchtimeQuery.queryItemValue("docid") },
            { "ei", playerWatchtimeQuery.queryItemValue("ei") },
            { "plid", playerWatchtimeQuery.queryItemValue("plid") },
            { "sdetail", playerWatchtimeQuery.queryItemValue("sdetail") },
            { "of", playerWatchtimeQuery.queryItemValue("of") },
            { "vm", playerWatchtimeQuery.queryItemValue("vm") }
        };
        outWatchtimeQuery.setQueryItems(map);
        outWatchtimeUrl.setQuery(outWatchtimeQuery);

        Http http;
        StatsUtils::setNeededHeaders(http, m_context, m_authStore);
        http.get(outWatchtimeUrl);
    }
    else if (url.path() == "/api/stats/playback")
    {
        info.block(true);
        if (!qtTubeApp->settings().playbackTracking) return;

        QUrlQuery playbackQuery(url);
        QUrlQuery playerPlaybackQuery(QUrl(m_playerResponse.playbackTracking.videostatsPlaybackUrl));

        QUrl outPlaybackUrl("https://www.youtube.com/api/stats/playback");
        QUrlQuery outPlaybackQuery;
        QList<QPair<QString, QString>> map =
        {
            { "ns", "yt" },
            { "el", "detailpage" },
            { "cpn", playbackQuery.queryItemValue("cpn") },
            { "ver", "2" },
            { "fmt", playbackQuery.queryItemValue("fmt") },
            { "fs", "0" },
            { "rt", playbackQuery.queryItemValue("rt") },
            { "euri", "" },
            { "lact", playbackQuery.queryItemValue("lact") },
            { "cl", playerPlaybackQuery.queryItemValue("cl") },
            { "mos", "0" },
            { "volume", playbackQuery.queryItemValue("volume") },
            { "cbr", m_context->client.browserName },
            { "cbrver", m_context->client.browserVersion },
            { "c", QString::number(static_cast<int>(m_context->client.clientType)) },
            { "cver", m_context->client.clientVersion },
            { "cplayer", "UNIPLAYER" },
            { "cos", m_context->client.osName },
            { "cosver", m_context->client.osVersion },
            { "cplatform", m_context->client.platform },
            { "hl", playbackQuery.queryItemValue("hl") },
            { "cr", playbackQuery.queryItemValue("cr") },
            { "uga", playerPlaybackQuery.queryItemValue("uga") },
            { "len", playbackQuery.queryItemValue("len") },
            { "fexp", playerPlaybackQuery.queryItemValue("fexp") },
            { "rtn", playbackQuery.queryItemValue("rtn") },
            { "afmt", playbackQuery.queryItemValue("afmt") },
            { "muted", playbackQuery.queryItemValue("muted") },
            { "docid", playbackQuery.queryItemValue("docid") },
            { "ei", playerPlaybackQuery.queryItemValue("ei") },
            { "plid", playerPlaybackQuery.queryItemValue("plid") },
            { "sdetail", playerPlaybackQuery.queryItemValue("sdetail") },
            { "of", playerPlaybackQuery.queryItemValue("of") },
            { "vm", playerPlaybackQuery.queryItemValue("vm") }
        };
        outPlaybackQuery.setQueryItems(map);
        outPlaybackUrl.setQuery(outPlaybackQuery);

        Http http;
        StatsUtils::setNeededHeaders(http, m_context, m_authStore);
        http.get(outPlaybackUrl);
    }
}
