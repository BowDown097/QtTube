#ifndef CHANNELVIEW_H
#define CHANNELVIEW_H
#include "httpreply.h"
#include "innertube/responses/browse/channelresponse.h"
#include "ui/widgets/subscribe/subscribewidget.h"
#include <QStackedWidget>
#include <QTabWidget>

class ChannelView : public QWidget
{
    Q_OBJECT
public:
    explicit ChannelView(const QString& channelId);
    ~ChannelView();
    void hotLoadChannel(const QString& channelId);
private:
    QLabel* channelBanner;
    QHBoxLayout* channelHeader;
    QWidget* channelHeaderWidget;
    QLabel* channelIcon;
    QString channelId;
    TubeLabel* channelName;
    QTabWidget* channelTabs;
    TubeLabel* handleAndVideos;
    QHBoxLayout* metaHbox;
    QVBoxLayout* metaVbox;
    QVBoxLayout* pageLayout;
    SubscribeWidget* subscribeWidget;

    void setTabsAndStyles(const InnertubeEndpoints::ChannelResponse& channelResp);
private slots:
    void setBanner(const HttpReply& reply);
    void setIcon(const HttpReply& reply);
};

#endif // CHANNELVIEW_H
