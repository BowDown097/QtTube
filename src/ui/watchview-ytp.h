#ifndef USEMPV
#ifndef WATCHVIEWYTP_H
#define WATCHVIEWYTP_H
#include "clickablelabel.h"
#include "httpreply.h"
#include "innertube/endpoints/video/next.h"
#include "innertube/endpoints/video/player.h"
#include "webengineplayer.h"
#include <QListWidget>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>

class WatchView : public QWidget
{
    Q_OBJECT
public:
    static WatchView* instance();
    void initialize(QStackedWidget* stackedWidget);
    void loadVideo(const InnertubeEndpoints::Next& next, const InnertubeEndpoints::Player& player, int progress = 0);
public slots:
    void goBack();
private:
    ClickableLabel* channelIcon;
    ClickableLabel* channelName;
    QVBoxLayout* hboxVbox;
    QHBoxLayout* primaryInfoHbox;
    QWidget* primaryInfoWidget;
    QListWidget* recommendations;
    QStackedWidget* stackedWidget;
    QLabel* subscribersLabel; // TODO: make into Hitchhiker-like subscribe button
    QLabel* titleLabel;
    WebEnginePlayer* wePlayer;

    WatchView(QWidget* parent = nullptr);
    QSize calcPlayerSize();
    void resizeEvent(QResizeEvent*) override;
    void setSubscriberCount(const InnertubeObjects::VideoSecondaryInfo& secondaryInfo);
private slots:
    void setChannelIcon(const HttpReply& reply);
};

#endif // WATCHVIEWYTP_H
#endif // USEMPV
