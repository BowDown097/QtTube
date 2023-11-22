#ifndef CHANNELVIEW_H
#define CHANNELVIEW_H
#include <QWidget>

namespace InnertubeEndpoints { class ChannelResponse; }

class HttpReply;
class QHBoxLayout;
class QLabel;
class QTabWidget;
class QVBoxLayout;
class SubscribeWidget;
class TubeLabel;

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
