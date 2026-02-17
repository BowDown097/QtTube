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
    TubeLabel* m_channelBanner;
    QWidget* m_channelHeaderContainer;
    QHBoxLayout* m_channelHeaderLayout;
    TubeLabel* m_channelIcon;
    QString m_channelId;
    TubeLabel* m_channelNameLabel;
    QTabWidget* m_channelTabs;
    QHBoxLayout* m_metaHbox;
    QVBoxLayout* m_metaVbox;
    QVBoxLayout* m_pageLayout;
    PluginData* m_plugin;
    SubscribeWidget* m_subscribeWidget;
    TubeLabel* m_subtextLabel;

    void loadChannel(const QString& channelId);
private slots:
    void loadTab(std::any requestData, int index);
    void processData(const QtTubePlugin::ChannelData& data);
    void processHeader(const QtTubePlugin::ChannelHeader& header);
    void processTabs(const QList<QtTubePlugin::ChannelTabData>& tabs);
signals:
    void loadFailed(const QtTubePlugin::Exception& ex);
};
