#include "webengineplayer.h"
#include "settingsstore.h"
#include "webchannelinterface.h"
#include <QVBoxLayout>
#include <QWebChannel>
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
    m_view->page()->setWebChannel(channel);
    channel->registerObject("interface", m_interface);
    channel->registerObject("settings", SettingsStore::instance());

    loadScriptFile(":/qtwebchannel/qwebchannel.js", QWebEngineScript::DocumentCreation);
    loadScriptFile(":/player/annotationlib/AnnotationParser.js", QWebEngineScript::DocumentReady);
    loadScriptFile(":/player/annotationlib/AnnotationRenderer.js", QWebEngineScript::DocumentReady);
    loadScriptFile(":/player/annotations.js", QWebEngineScript::DocumentReady);
    loadScriptFile(":/player/global.js", QWebEngineScript::DocumentReady);
    loadScriptFile(":/player/h264ify.js", QWebEngineScript::DocumentReady);
    loadScriptFile(":/player/integration.js", QWebEngineScript::DocumentReady);
    loadScriptFile(":/player/sponsorblock.js", QWebEngineScript::DocumentReady);

    QString annotationStylesData = getFileContents(":/player/annotationlib/AnnotationRenderer.css");
    QString patchesData = getFileContents(":/player/patches.js");
    QString stylesData = getFileContents(":/player/styles.css");
    loadScriptString(patchesData.arg(annotationStylesData + stylesData), QWebEngineScript::DocumentReady);

    m_view->page()->profile()->setUrlRequestInterceptor(m_interceptor);
    m_view->settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
    m_view->settings()->setAttribute(QWebEngineSettings::PlaybackRequiresUserGesture, false);
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
    m_view->load(QUrl(QStringLiteral("https://youtube.com/embed/%1?t=%2").arg(vId).arg(progress)));
}

void WebEnginePlayer::seek(int progress)
{
    m_view->page()->runJavaScript(QStringLiteral("document.getElementById('movie_player').seekTo(%1);").arg(progress));
}
