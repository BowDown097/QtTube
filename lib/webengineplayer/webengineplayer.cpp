#include "webengineplayer.h"
#include "settingsstore.h"
#include "webchannelinterface.h"
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
    channel->registerObject("interface", WebChannelInterface::instance());

    loadScriptFile(":/qtwebchannel/qwebchannel.js", QWebEngineScript::DocumentCreation);
    loadScriptFile(":/player/global.js", QWebEngineScript::DocumentReady);
    loadScriptFile(":/player/integration.js", QWebEngineScript::DocumentReady);
    loadScriptFile(":/player/sponsorblock.js", QWebEngineScript::DocumentReady);

    QString patchesData = getFileContents(":/player/patches.js");
    QString stylesData = getFileContents(":/player/styles.css");
    loadScriptString(patchesData.arg(stylesData), QWebEngineScript::DocumentReady);

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
