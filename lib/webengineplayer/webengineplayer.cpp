#include "webengineplayer.h"
#include "settingsstore.h"
#include "webchannelmethods.h"
#include <QVBoxLayout>
#include <QWebChannel>
#include <QWebEngineProfile>
#include <QWebEngineScriptCollection>
#include <QWebEngineSettings>

WebEnginePlayer::WebEnginePlayer(QWidget* parent)
    : QWidget(parent), m_interceptor(new PlayerInterceptor(this)), m_view(new QWebEngineView(this))
{
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(m_view);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    QWebChannel* channel = new QWebChannel(m_view->page());
    m_view->page()->setWebChannel(channel);
    channel->registerObject("methods", WebChannelMethods::instance());

    QWebEngineScript webChannelJs;
    webChannelJs.setInjectionPoint(QWebEngineScript::DocumentCreation);
    webChannelJs.setWorldId(QWebEngineScript::MainWorld);
    QFile webChannelJsFile(":/qtwebchannel/qwebchannel.js");
    webChannelJsFile.open(QFile::ReadOnly);
    webChannelJs.setSourceCode(webChannelJsFile.readAll());
    m_view->page()->scripts().insert(webChannelJs);

    QWebEngineScript inject;
    inject.setInjectionPoint(QWebEngineScript::DocumentReady);
    inject.setWorldId(QWebEngineScript::MainWorld);
    inject.setSourceCode(m_playerJs);
    m_view->page()->scripts().insert(inject);

    m_view->page()->profile()->setUrlRequestInterceptor(m_interceptor);
    m_view->settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
    m_view->settings()->setAttribute(QWebEngineSettings::PlaybackRequiresUserGesture, false);
    connect(m_view->page(), &QWebEnginePage::fullScreenRequested, this, &WebEnginePlayer::fullScreenRequested);
}

void WebEnginePlayer::play(const QString& vId, int progress)
{
    QString sbc = QJsonDocument(QJsonArray::fromStringList(SettingsStore::instance().sponsorBlockCategories)).toJson(QJsonDocument::Compact);
    QString q = QMetaEnum::fromType<SettingsStore::PlayerQuality>().valueToKey(SettingsStore::instance().preferredQuality);
    m_view->load(QUrl(QStringLiteral("https://youtube.com/embed/%1?sbc=%2&q=%3&t=%4&v=%5")
                      .arg(vId, sbc, q.toLower())
                      .arg(progress)
                      .arg(SettingsStore::instance().preferredVolume)));
}

void WebEnginePlayer::seek(int progress)
{
    m_view->page()->runJavaScript(QStringLiteral("document.getElementById(\"movie_player\").seekTo(%1);").arg(progress));
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
