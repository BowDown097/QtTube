#pragma once
#include <QWidget>

namespace InnertubeEndpoints { class ChannelResponse; }

namespace InnertubeObjects
{
    class ChannelC4Header;
    class ChannelPageHeader;
    class EntityMutation;
    class ResponsiveImage;
}

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
    void prepareAvatarAndBanner(const InnertubeObjects::ResponsiveImage& avatar,
                                const InnertubeObjects::ResponsiveImage& banner);
    void prepareHeader(const InnertubeObjects::ChannelC4Header& c4Header);
    void prepareHeader(const InnertubeObjects::ChannelPageHeader& pageHeader,
                       const QList<InnertubeObjects::EntityMutation>& mutations);
private slots:
    void loadTab(const InnertubeEndpoints::ChannelResponse& response, int index);
    void setBanner(const HttpReply& reply);
    void setIcon(const HttpReply& reply);
};
