#include "webengineplayer.h"
#include <QVBoxLayout>
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

    QWebEngineScript inject;
    inject.setInjectionPoint(QWebEngineScript::DocumentCreation);
    inject.setWorldId(QWebEngineScript::MainWorld);
    inject.setRunsOnSubFrames(true);
    inject.setSourceCode(m_playerJs);
    m_view->page()->scripts().insert(inject);

    m_view->page()->profile()->setUrlRequestInterceptor(m_interceptor);
    m_view->settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
    m_view->settings()->setAttribute(QWebEngineSettings::PlaybackRequiresUserGesture, false);
    connect(m_view->page(), &QWebEnginePage::fullScreenRequested, this, &WebEnginePlayer::fullScreenRequested);
}

void WebEnginePlayer::play(const QString& vId, int progress, bool showSBToasts, const QVariantList& sponsorBlockCategories)
{
    QString sbc = QJsonDocument(QJsonArray::fromVariantList(sponsorBlockCategories)).toJson(QJsonDocument::Compact);
    m_view->load(QUrl(QStringLiteral("https://youtube.com/embed/%1?t=%2&st=%3&sbc=%4")
                      .arg(vId)
                      .arg(progress)
                      .arg(showSBToasts)
                      .arg(sbc)));
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
