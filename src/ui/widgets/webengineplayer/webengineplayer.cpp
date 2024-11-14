#include "webengineplayer.h"
#include "fullscreenwindow.h"
#include "playerinterceptor.h"
#include "qttubeapplication.h"
#include "webchannelinterface.h"
#include <QBoxLayout>
#include <QFile>
#include <QWebChannel>
#include <QWebEngineCookieStore>
#include <QWebEngineProfile>
#include <QWebEngineScriptCollection>
#include <QWebEngineSettings>

WebEnginePlayer::WebEnginePlayer(QWidget* parent)
    : QWidget(parent),
      m_interceptor(new PlayerInterceptor(this)),
      m_interface(new WebChannelInterface(this)),
      m_view(new QWebEngineView(this))
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(m_view);
    layout->setContentsMargins(0, 0, 0, 0);

    QWebChannel* channel = new QWebChannel(m_view->page());
    channel->registerObject("interface", m_interface);
    channel->registerObject("settings", &qtTubeApp->settings());
    m_view->page()->setWebChannel(channel);

    loadScriptFile(":/qtwebchannel/qwebchannel.js", QWebEngineScript::DocumentCreation);
    loadScriptFile(":/player/annotationlib/AnnotationParser.js", QWebEngineScript::DocumentReady);
    loadScriptFile(":/player/annotationlib/AnnotationRenderer.js", QWebEngineScript::DocumentReady);
    loadScriptFile(":/player/annotations.js", QWebEngineScript::DocumentReady);
    loadScriptFile(":/player/global.js", QWebEngineScript::DocumentCreation);
    loadScriptFile(":/player/h264ify.js", QWebEngineScript::DocumentReady);
    loadScriptFile(":/player/integration.js", QWebEngineScript::DocumentReady);
    loadScriptFile(":/player/interceptors.js", QWebEngineScript::DocumentCreation);
    loadScriptFile(":/player/sponsorblock.js", QWebEngineScript::DocumentReady);

    QString annotationStylesData = getFileContents(":/player/annotationlib/AnnotationRenderer.css");
    QString patchesData = getFileContents(":/player/patches.js");
    QString stylesData = getFileContents(":/player/styles.css");
    loadScriptString(patchesData.arg(annotationStylesData + stylesData), QWebEngineScript::DocumentReady);

    m_view->page()->profile()->setUrlRequestInterceptor(m_interceptor);
    m_view->settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
    m_view->settings()->setAttribute(QWebEngineSettings::PlaybackRequiresUserGesture, false);

    connect(m_interface, &WebChannelInterface::progressChanged, this, &WebEnginePlayer::progressChanged);
    connect(m_view->page(), &QWebEnginePage::fullScreenRequested, this, &WebEnginePlayer::fullScreenRequested);
}

void WebEnginePlayer::fullScreenRequested(QWebEngineFullScreenRequest request)
{
    request.accept();
    if (request.toggleOn())
    {
        if (m_fullScreenWindow) return;
        m_fullScreenWindow.reset(new FullScreenWindow(m_view));
    }
    else
    {
        if (!m_fullScreenWindow) return;
        m_fullScreenWindow.reset();
    }
}

QString WebEnginePlayer::getFileContents(const QString& path)
{
    QFile file(path);
    file.open(QFile::ReadOnly);
    return file.readAll();
}

void WebEnginePlayer::loadScriptFile(const QString& path, QWebEngineScript::InjectionPoint injectionPoint)
{
    loadScriptString(getFileContents(path), injectionPoint);
}

void WebEnginePlayer::loadScriptString(const QString& data, QWebEngineScript::InjectionPoint injectionPoint)
{
    QWebEngineScript script;
    script.setInjectionPoint(injectionPoint);
    script.setSourceCode(data);
    script.setWorldId(QWebEngineScript::MainWorld);
    m_view->page()->scripts().insert(script);
}

void WebEnginePlayer::play(const QString& vId, int progress)
{
    // h264 settings must be passed as a parameter because
    // the video format is determined before QWebChannel loads
    m_view->load(QUrl(QStringLiteral("https://youtube.com/embed/%1?t=%2&h264Only=%3&no60Fps=%4&adblock=%5")
                          .arg(vId)
                          .arg(progress)
                          .arg(qtTubeApp->settings().h264Only)
                          .arg(qtTubeApp->settings().disable60Fps)
                          .arg(qtTubeApp->settings().blockAds)));
}

void WebEnginePlayer::setAuthStore(InnertubeAuthStore* authStore)
{
    m_interceptor->setAuthStore(authStore);
    if (!authStore->populated())
        return;

    // add cookies from auth store to the video page.
    // these are required should the user wish to watch an age restricted video,
    // as the last surviving age restriction bypass was patched on 10/21/2024.
    const QStringList cookiePairs = authStore->toCookieString().split(';', Qt::SkipEmptyParts);
    QWebEngineCookieStore* cookieStore = m_view->page()->profile()->cookieStore();

    for (const QString& cookiePair : cookiePairs)
    {
        QStringList parts = cookiePair.split('=');
        QNetworkCookie cookie(parts[0].toUtf8(), parts[1].toUtf8());
        cookie.setDomain("www.youtube.com");
        cookie.setPath("/");
        cookieStore->setCookie(cookie);
    }
}

void WebEnginePlayer::setContext(InnertubeContext* context)
{ m_interceptor->setContext(context); }

void WebEnginePlayer::setPlayerResponse(const InnertubeEndpoints::PlayerResponse& resp)
{ m_interceptor->setPlayerResponse(resp); }

void WebEnginePlayer::seek(int progress)
{
    m_view->page()->runJavaScript(QStringLiteral("document.getElementById('movie_player').seekTo(%1);").arg(progress));
}
