#ifdef USEMPV
#ifndef WATCHVIEWMPV_H
#define WATCHVIEWMPV_H
#include "innertube/endpoints/video/next.h"
#include "innertube/endpoints/video/player.h"
#include "innertube/itc-objects/innertubeclient.h"
#include "lib/media/media.h"
#include <QGridLayout>
#include <QListWidget>
#include <QStackedWidget>
#include <QTimer>

class WatchView : public QWidget
{
    Q_OBJECT
public:
    static WatchView* instance();
    void initialize(const InnertubeClient& client, QStackedWidget* stackedWidget);
    void loadVideo(const InnertubeEndpoints::Next& next, const InnertubeEndpoints::Player& player, int progress = 0);
private slots:
    void mediaStateChanged(Media::State state);
    void volumeChanged(double volume);
private:
    WatchView(QWidget* parent = nullptr);
    static QString getCpn();
    static void reportPlayback(const InnertubeClient& client, const InnertubeEndpoints::Player& player);
    static void reportWatchtime(const InnertubeClient& client, const InnertubeEndpoints::Player& player, long long position);
    QGridLayout* grid;
    InnertubeClient itc;
    Media* media;
    QListWidget* recommendations;
    QStackedWidget* stackedWidget;
    QTimer* watchtimeTimer;
};

#endif // WATCHVIEWMPV_H
#endif // USEMPV
