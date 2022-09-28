#ifndef WEBENGINEPLAYER_H
#define WEBENGINEPLAYER_H
#include "endpoints/video/player.h"
#include "itc-objects/innertubeauthstore.h"
#include "fullscreenwindow.h"
#include <QMainWindow>
#include <QWebEngineFullScreenRequest>
#include <QWebEngineView>

class WebEnginePlayer : public QWidget
{
    Q_OBJECT
public:
    explicit WebEnginePlayer(InnertubeContext* context, InnertubeAuthStore* authStore, const InnertubeEndpoints::Player& player, QWidget* parent = nullptr);
public slots:
    void play(const QString& vId);
    void playAuthorUploads(const QString& aId);
    void playPlaylist(const QString& pId);
    void reset();
private slots:
    void fullScreenRequested(QWebEngineFullScreenRequest request);
private:
    QScopedPointer<FullScreenWindow> m_fullScreenWindow;
    QWebEngineView* m_view = nullptr;
};

#endif // WEBENGINEPLAYER_H
