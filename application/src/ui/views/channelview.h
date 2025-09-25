#pragma once
#include "qttube-plugin/components/reply.h"
#include "qttube-plugin/components/replytypes/channeldata.h"
#include <QWidget>

struct PluginData;
class QHBoxLayout;
class QTabWidget;
class QVBoxLayout;
class SubscribeWidget;
class TubeLabel;

class ChannelView : public QWidget
{
    Q_OBJECT
public:
    explicit ChannelView(const QString& channelId, PluginData* plugin);
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
    PluginData* plugin;
    SubscribeWidget* subscribeWidget;

    void loadChannel(const QString& channelId);
private slots:
    void loadTab(std::any requestData, int index);
    void processData(const QtTubePlugin::ChannelData& data);
    void processHeader(const QtTubePlugin::ChannelHeader& header);
    void processTabs(const QList<QtTubePlugin::ChannelTabData>& tabs);
signals:
    void loadFailed(const QtTubePlugin::Exception& ex);
};
