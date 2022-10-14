#ifndef WEBENGINEPLAYER_H
#define WEBENGINEPLAYER_H
#include "innertube/endpoints/video/player.h"
#include "innertube/itc-objects/innertubeauthstore.h"
#include "fullscreenwindow.h"
#include <QWebEngineFullScreenRequest>

class WebEnginePlayer : public QWidget
{
    Q_OBJECT
public:
    WebEnginePlayer(InnertubeContext* context, InnertubeAuthStore* authStore, const InnertubeEndpoints::Player& player,
                    bool playbackTracking, bool watchtimeTracking, QWidget* parent = nullptr);
public slots:
    void play(const QString& vId, int progress, bool showSBToasts, const QVariantList& sponsorBlockCategories);
    // void playAuthorUploads(const QString& aId);
    // void playPlaylist(const QString& pId);
    void reset();
private slots:
    void fullScreenRequested(QWebEngineFullScreenRequest request);
private:
    QScopedPointer<FullScreenWindow> m_fullScreenWindow;
    QWebEngineView* m_view = nullptr;
};

#endif // WEBENGINEPLAYER_H
