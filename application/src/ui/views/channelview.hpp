#pragma once
#include "plugins/pluginentry.hpp"
#include <qttube-plugin/components/reply.h>
#include <qttube-plugin/components/replytypes/channeldata.h>
#include <QWidget>

class ChannelTabWidget;
class QHBoxLayout;
class QVBoxLayout;
class SubscribeWidget;
class TubeLabel;

class ChannelView : public QWidget
{
    Q_OBJECT
public:
    explicit ChannelView(const QString& channelId, PluginEntry* plugin);
    ~ChannelView();
    void hotLoadChannel(const QString& channelId);
private:
    TubeLabel* m_avatar;
    TubeLabel* m_banner;
    QString m_channelId;
    QWidget* m_headerContainer;
    QHBoxLayout* m_headerLayout;
    QHBoxLayout* m_metaHbox;
    QVBoxLayout* m_metaVbox;
    TubeLabel* m_nameLabel;
    QVBoxLayout* m_pageLayout;
    PluginEntry* m_plugin;
    SubscribeWidget* m_subscribeWidget;
    TubeLabel* m_subtextLabel;
    ChannelTabWidget* m_tabWidget;

    void loadChannel(const QString& channelId);
private slots:
    void processData(const QtTubePlugin::ChannelData& data);
signals:
    void loadFailed(const QtTubePlugin::Exception& ex);
};
