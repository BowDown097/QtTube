#ifdef USEMPV
#ifndef WATCHVIEWMPV_H
#define WATCHVIEWMPV_H
#include "clickablelabel.h"
#include "innertube/itc-objects/innertubeclient.h"
#include "innertube/responses/video/nextresponse.h"
#include "innertube/responses/video/playerresponse.h"
#include "lib/media/media.h"
#include <QStackedWidget>
#include <QTimer>

class WatchView : public QWidget
{
    Q_OBJECT
public:
    static WatchView* instance();
    void initialize(const InnertubeClient& client, QStackedWidget* stackedWidget);
    void loadVideo(const QString& videoId, int progress = 0);
public slots:
    void goBack();
private slots:
    void mediaStateChanged(Media::State state);
    void volumeChanged(double volume);
private:
    ClickableLabel* channelIcon = nullptr;
    ClickableLabel* channelName = nullptr;
    InnertubeClient itc;
    Media* media = nullptr;
    QVBoxLayout* pageLayout = nullptr;
    QHBoxLayout* primaryInfoHbox = nullptr;
    QVBoxLayout* primaryInfoVbox = nullptr;
    QWidget* primaryInfoWrapper = nullptr;
    QStackedWidget* stackedWidget = nullptr;
    QLabel* subscribersLabel = nullptr; // TODO: make into Hitchhiker-like subscribe button
    QLabel* titleLabel = nullptr;
    QTimer* watchtimeTimer = nullptr;

    explicit WatchView(QWidget* parent = nullptr) : QWidget(parent) {}
    QString getCpn();
    void reportPlayback(const InnertubeEndpoints::PlayerResponse& playerResp);
    void reportWatchtime(const InnertubeEndpoints::PlayerResponse& playerResp, long long position);
    void resizeEvent(QResizeEvent*) override;
};

#endif // WATCHVIEWMPV_H
#endif // USEMPV
