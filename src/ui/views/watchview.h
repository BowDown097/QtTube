#ifndef WATCHVIEWYTP_H
#define WATCHVIEWYTP_H
#include "httpreply.h"
#include "innertube/endpoints/video/next.h"
#include "innertube/endpoints/video/player.h"
#include "innertube/responses/video/updatedmetadataresponse.h"
#include "ui/widgets/channellabel.h"
#include "ui/widgets/iconlabel.h"
#include "ui/widgets/subscribewidget.h"
#include "ui/widgets/tubelabel.h"
#include <QProgressBar>
#include <QScrollArea>
#include <QSpacerItem>
#include <QTimer>
#include <QVBoxLayout>

#ifdef USEMPV
#include "lib/media/media.h"
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
    void hotLoadVideo(const QString& videoId, int progress = 0);
    void loadVideo(const QString& videoId, int progress = 0);
public slots:
    void goBack();
private slots:
    void copyChannelUrl();
    void descriptionLinkActivated(const QString& url);
    void processNext(const InnertubeEndpoints::Next& endpoint);
    void processPlayer(const InnertubeEndpoints::Player& endpoint);
    void showContextMenu(const QPoint& pos);
    void toggleShowMore();
private:
    WatchView() {}
    static inline WatchView* m_watchView;

    TubeLabel* channelIcon;
    QString channelId;
    ChannelLabel* channelLabel;
    TubeLabel* date;
    TubeLabel* description;
    IconLabel* dislikeLabel;
    QFrame* frame;
    QSpacerItem* infoSpacer;
    QProgressBar* likeBar;
    QHBoxLayout* likeBarWrapper;
    IconLabel* likeLabel;
    QVBoxLayout* menuVbox;
    QWidget* menuWrapper;
    QTimer* metadataUpdateTimer = nullptr;
    QHBoxLayout* primaryInfoHbox;
    QVBoxLayout* primaryInfoVbox;
    QWidget* primaryInfoWrapper;
    QScrollArea* scrollArea;
    TubeLabel* showMoreLabel;
    QHBoxLayout* subscribeHbox;
    SubscribeWidget* subscribeWidget;
    TubeLabel* subscribersLabel;
    TubeLabel* titleLabel;
    QHBoxLayout* topLevelButtons;
    TubeLabel* viewCount;

#ifdef USEMPV
    Media* media = nullptr;
    QTimer* watchtimeTimer = nullptr;
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
    void updateMetadata(const InnertubeEndpoints::UpdatedMetadataResponse& resp);

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
