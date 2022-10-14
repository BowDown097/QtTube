#include "playerinterceptor.h"
#include "webengineplayer.h"
#include <QVBoxLayout>
#include <QWebEngineSettings>

WebEnginePlayer::WebEnginePlayer(InnertubeContext* context, InnertubeAuthStore* authStore, const InnertubeEndpoints::Player& player,
    bool playbackTracking, bool watchtimeTracking, QWidget* parent) : QWidget(parent), m_view(new QWebEngineView(this))
{
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(m_view);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    PlayerInterceptor* interceptor = new PlayerInterceptor(context, authStore, player, playbackTracking, watchtimeTracking);
    m_view->page()->profile()->setUrlRequestInterceptor(interceptor);
    m_view->settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
    m_view->settings()->setAttribute(QWebEngineSettings::PlaybackRequiresUserGesture, false);
    connect(m_view->page(), &QWebEnginePage::fullScreenRequested, this, &WebEnginePlayer::fullScreenRequested);

    reset();
}

void WebEnginePlayer::play(const QString& vId, int progress, bool showSBToasts, const QVariantList& sponsorBlockCategories)
{
    QString sbc = QJsonDocument(QJsonArray::fromVariantList(sponsorBlockCategories)).toJson();
    m_view->load(QUrl(QStringLiteral("https://thughunting.party/ytp/?v=%1&t=%2&st=%3&sbc=%4").arg(vId).arg(progress).arg(showSBToasts).arg(sbc)));
}

/*
void WebEnginePlayer::playAuthorUploads(const QString& aId)
{
    m_view->load(QUrl(QStringLiteral("https://thughunting.party/ytp/?ap=%1").arg(aId)));
}

void WebEnginePlayer::playPlaylist(const QString& pId)
{
    m_view->load(QUrl(QStringLiteral("https://thughunting.party/ytp/?p=%1").arg(pId)));
}
*/

void WebEnginePlayer::reset()
{
    m_view->setHtml("<html><head><style type='text/css'>html{overflow: hidden;}"
                    "body{ height: 100%;width: 100%; background-color: black;}"
                    "</style></head><body></body></html>");
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
