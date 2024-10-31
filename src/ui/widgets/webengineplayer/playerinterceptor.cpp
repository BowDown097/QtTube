#include "playerinterceptor.h"
#include "qttubeapplication.h"
#include "utils/tubeutils.h"
#include <QUrlQuery>

void PlayerInterceptor::interceptRequest(QWebEngineUrlRequestInfo& info)
{
    if (!m_authStore || !m_context)
    {
        qDebug() << "Auth store or InnerTube context is null - using YouTube's default tracking.";
        return;
    }

    const QUrl url = info.requestUrl();
    if (isTrackingUrl(url))
    {
        info.block(true);
        return;
    }

    if (url.path() == "/api/stats/watchtime")
        doWatchtime(info);
    else if (url.path() == "/api/stats/playback")
        doPlayback(info);
}

void PlayerInterceptor::doWatchtime(QWebEngineUrlRequestInfo& info)
{
    info.block(true);
    if (qtTubeApp->settings().watchtimeTracking)
        TubeUtils::reportWatchtime(m_playerResponse, QUrlQuery(info.requestUrl()).queryItemValue("st").toLongLong());
}

void PlayerInterceptor::doPlayback(QWebEngineUrlRequestInfo& info)
{
    info.block(true);
    if (qtTubeApp->settings().playbackTracking)
        TubeUtils::reportPlayback(m_playerResponse);
}

bool PlayerInterceptor::isTrackingUrl(const QUrl& url)
{
    return url.host().contains("doubleclick.net") || url.host() == "jnn-pa.googleapis.com" ||
           url.path() == "/youtubei/v1/log_event" || url.path() == "/api/stats/qoe" ||
           url.path() == "/ptracking" || url.toString().contains("play.google.com/log");
}
