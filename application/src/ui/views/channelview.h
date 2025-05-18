#pragma once
#include <QWidget>

namespace InnertubeEndpoints { struct ChannelResponse; }

namespace InnertubeObjects
{
    struct ChannelC4Header;
    struct ChannelPageHeader;
    struct EntityMutation;
    struct ResponsiveImage;
}

class QHBoxLayout;
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
    TubeLabel* channelBanner;
    QWidget* channelHeaderContainer;
    QHBoxLayout* channelHeaderLayout;
    TubeLabel* channelIcon;
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
};
