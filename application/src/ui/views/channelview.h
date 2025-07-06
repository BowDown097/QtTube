#pragma once
#include "qttube-plugin/components/reply.h"
#include "qttube-plugin/components/replytypes/channeldata.h"
#include <QWidget>

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
private slots:
    void loadTab(std::any requestData, int index);
    void processData(const QtTube::ChannelData& data);
    void processHeader(const QtTube::ChannelHeader& header);
    void processTabs(const QList<QtTube::ChannelTabData>& tabs);
signals:
    void loadFailed(const QtTube::PluginException& ex);
};
