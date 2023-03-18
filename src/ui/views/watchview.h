#ifndef WATCHVIEWYTP_H
#define WATCHVIEWYTP_H
#include "httpreply.h"
#include "innertube/endpoints/video/next.h"
#include "innertube/endpoints/video/player.h"
#include "innertube/innertubeexception.h"
#include "innertube/responses/video/updatedmetadataresponse.h"
#include "ui/widgets/labels/channellabel.h"
#include "ui/widgets/labels/iconlabel.h"
#include "ui/widgets/subscribe/subscribewidget.h"
#include <QProgressBar>
#include <QScrollArea>
#include <QSpacerItem>
#include <QTimer>

#ifdef USEMPV
#include "lib/media/media.h"
#else
#include "ui/widgets/webengineplayer/webengineplayer.h"
#endif

#ifdef Q_OS_MACOS
#include <IOKit/pwr_mgt/IOPMLib.h>
#endif

class WatchView : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(WatchView)
public:
    explicit WatchView(const QString& videoId, int progress = 0, QWidget* parent = nullptr);
    ~WatchView();
    void hotLoadVideo(const QString& videoId, int progress = 0);
private slots:
    void copyChannelUrl();
    void descriptionLinkActivated(const QString& url);
    void likeOrDislike(bool like, const InnertubeObjects::ToggleButton& toggleButton);
    void processNext(const InnertubeEndpoints::Next& endpoint);
    void processPlayer(const InnertubeEndpoints::Player& endpoint);
    void showContextMenu(const QPoint& pos);
    void toggleShowMore();
protected:
    void resizeEvent(QResizeEvent* event) override;
signals:
    void loadFailed(const InnertubeException& ie);
    void navigateChannelRequested(const QString& channelId);
private:
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
    QPointer<QTimer> metadataUpdateTimer;
    QHBoxLayout* primaryInfoHbox;
    QVBoxLayout* primaryInfoVbox;
    QWidget* primaryInfoWrapper;
    QScrollArea* scrollArea;
    TubeLabel* showMoreLabel;
    SubscribeWidget* subscribeWidget;
    TubeLabel* titleLabel;
    QHBoxLayout* topLevelButtons;
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

    QSize calcPlayerSize(QSize maxSize);
    QString generateFormattedDescription(const InnertubeObjects::InnertubeString& description);
    void setChannelIcon(const HttpReply& reply);
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
