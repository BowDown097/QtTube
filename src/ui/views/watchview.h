#ifndef WATCHVIEWYTP_H
#define WATCHVIEWYTP_H
#include "httpreply.h"
#include "innertube/endpoints/video/next.h"
#include "innertube/endpoints/video/player.h"
#include "innertube/objects/video/secondaryinfo/videosecondaryinfo.h"
#include "ui/widgets/subscribewidget.h"
#include "ui/widgets/tubelabel.h"
#include <QProgressBar>
#include <QStackedWidget>
#include <QVBoxLayout>

#ifdef USEMPV
#include "innertube/responses/video/playerresponse.h"
#include "lib/media/media.h"
#include <QTimer>
#else
#include "webengineplayer.h"
#endif

#ifdef Q_OS_MACOS
#include <IOKit/pwr_mgt/IOPMLib.h>
#endif

class WatchView : public QWidget
{
    Q_OBJECT
public:
    Q_DISABLE_COPY(WatchView)
    static WatchView* instance();
    void loadVideo(const QString& videoId, int progress = 0);
public slots:
    void goBack();
private slots:
    void copyChannelUrl();
    void processNext(const InnertubeEndpoints::Next& endpoint);
    void processPlayer(const InnertubeEndpoints::Player& endpoint);
    void showContextMenu(const QPoint& pos);
private:
    WatchView() {}
    static inline WatchView* m_watchView;

    TubeLabel* channelIcon;
    QString channelId;
    TubeLabel* channelName;
    QProgressBar* likeBar;
    QHBoxLayout* likeBarWrapper;
    TubeLabel* likesDislikesLabel;
    QVBoxLayout* menuVbox;
    QWidget* menuWrapper;
    QVBoxLayout* pageLayout;
    QHBoxLayout* primaryInfoHbox;
    QVBoxLayout* primaryInfoVbox;
    QWidget* primaryInfoWrapper;
    QHBoxLayout* subscribeHbox;
    SubscribeWidget* subscribeWidget;
    TubeLabel* subscribersLabel;
    TubeLabel* titleLabel;
    QString currentVideoId;
    TubeLabel* viewCount;

#ifdef USEMPV
    Media* media;
    QTimer* watchtimeTimer;
#else
    WebEnginePlayer* wePlayer;
#endif

#ifdef Q_OS_MACOS
    inline static IOPMAssertionID sleepAssert;
#endif

    void resizeEvent(QResizeEvent* event) override; // webengine views don't resize automatically
    QSize calcPlayerSize();
    void navigateChannel();
    void setChannelIcon(const HttpReply& reply);
    void setSubscriberCount(const InnertubeObjects::VideoSecondaryInfo& secondaryInfo);
    void toggleIdleSleep(bool toggle);

#ifdef USEMPV
    QString getCpn();
    void reportPlayback(const InnertubeEndpoints::PlayerResponse& playerResp);
    void reportWatchtime(const InnertubeEndpoints::PlayerResponse& playerResp, long long position);
private slots:
    void mediaStateChanged(Media::State state);
    void volumeChanged(double volume);
#endif
};

#endif // WATCHVIEWYTP_H
