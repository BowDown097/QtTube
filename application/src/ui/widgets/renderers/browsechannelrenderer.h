#pragma once
#include "qttube-plugin/objects/channel.h"
#include <QWidget>

class ChannelLabel;
struct PluginData;
class QHBoxLayout;
class QVBoxLayout;
class SubscribeWidget;
class TubeLabel;

class BrowseChannelRenderer : public QWidget
{
    Q_OBJECT
public:
    explicit BrowseChannelRenderer(PluginData* plugin, QWidget* parent = nullptr);
    void setData(const QtTubePlugin::Channel& channel);
private:
    QString m_channelId;
    TubeLabel* m_descriptionLabel;
    QHBoxLayout* m_layout;
    TubeLabel* m_metadataLabel;
    SubscribeWidget* m_subscribeWidget;
    QVBoxLayout* m_textLayout;
    TubeLabel* m_thumbLabel;
    ChannelLabel* m_titleLabel;
};
