#include "youtubeplayer.h"
#include "innertube.h"
#include "youtubeplugin.h"
#include <QWebChannel>
#include <QWebEngineCookieStore>
#include <QWebEngineProfile>

void PlayerInterceptor::interceptRequest(QWebEngineUrlRequestInfo& info)
{
    const QUrl url = info.requestUrl();
    if (isTrackingUrl(url))
    {
        info.block(true);
        return;
    }

    if (url.path() == "/api/stats/watchtime")
        info.block(!g_settings->watchtimeTracking);
    else if (url.path() == "/api/stats/playback")
        info.block(!g_settings->playbackTracking);
}

bool PlayerInterceptor::isTrackingUrl(const QUrl& url)
{
    return url.host().contains("doubleclick.net") || url.host() == "jnn-pa.googleapis.com" ||
           url.path() == "/youtubei/v1/log_event" || url.path() == "/api/stats/qoe" ||
           url.path() == "/ptracking" || url.toString().contains("play.google.com/log");
}

QString getFileContents(const QString& path)
{
    if (QFile file(path); file.open(QFile::ReadOnly))
        return file.readAll();
    else
        return QString();
}

YouTubePlayer::YouTubePlayer(QWidget* parent)
    : QtTubePlugin::WebPlayer(parent), m_interceptor(new PlayerInterceptor(this))
{
    m_channel->registerObject("pluginSettings", g_settings);

    loadScriptFile(":/annotationlib/AnnotationParser.js", QWebEngineScript::DocumentReady);
    loadScriptFile(":/annotationlib/AnnotationRenderer.js", QWebEngineScript::DocumentReady);
    loadScriptFile(":/annotations.js", QWebEngineScript::DocumentReady);
    loadScriptFile(":/integration.js", QWebEngineScript::DocumentReady);
    loadScriptFile(":/interceptors.js", QWebEngineScript::DocumentCreation);
    loadScriptFile(":/patches.js", QWebEngineScript::DocumentReady);
    loadScriptFile(":/sponsorblock.js", QWebEngineScript::DocumentReady);

    loadStyleFile(":/annotationlib/AnnotationRenderer.css");
    loadStyleFile(":/styles.css");

    m_view->page()->profile()->setUrlRequestInterceptor(m_interceptor);

    // add cookies from auth store to the video page.
    // these are required should the user wish to watch an age restricted video,
    // as the last surviving age restriction bypass was patched on 10/21/2024.
    const QStringList cookiePairs = InnerTube::instance()->authStore()->toCookieString().split(';', Qt::SkipEmptyParts);
    QWebEngineCookieStore* cookieStore = m_view->page()->profile()->cookieStore();

    for (const QString& cookiePair : cookiePairs)
    {
        QStringList parts = cookiePair.split('=');
        QNetworkCookie cookie(parts[0].toUtf8(), parts[1].toUtf8());
        cookie.setDomain("youtube.com");
        cookie.setPath("/");
        cookieStore->setCookie(cookie);
    }
}

void YouTubePlayer::play(const QString& videoId, int progress, QtTubePlugin::PlayerSettings* settings)
{
    m_channel->registerObject("playerSettings", settings);
    // h264 settings must be passed as a parameter because
    // the video format is determined before QWebChannel loads
    m_view->load(QUrl(QStringLiteral("https://youtube.com/embed/%1?t=%2&h264Only=%3&adblock=%4")
        .arg(videoId).arg(progress).arg(settings->h264Only).arg(g_settings->blockAds)));
}

void YouTubePlayer::seek(int progress)
{
    m_view->page()->runJavaScript(QStringLiteral("document.getElementById('movie_player').seekTo(%1);").arg(progress));
}
