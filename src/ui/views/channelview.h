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
    QWidget* channelHeaderContainer;
    QHBoxLayout* channelHeaderLayout;
    QLabel* channelIcon;
    QString channelId;
    TubeLabel* channelName;
    QTabWidget* channelTabs;
    TubeLabel* handleAndVideos;
    QHBoxLayout* metaHbox;
    QVBoxLayout* metaVbox;
    QVBoxLayout* pageLayout;
    SubscribeWidget* subscribeWidget;

    void loadChannel(const QString& channelId);
private slots:
    void loadTab(const InnertubeEndpoints::ChannelResponse& response, int index);
    void setBanner(const HttpReply& reply);
    void setIcon(const HttpReply& reply);
};

#endif // CHANNELVIEW_H
