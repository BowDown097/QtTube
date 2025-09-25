#pragma once
#include "qttube-plugin/objects/channel.h"
#include <QPointer>
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
    QString channelId;
    TubeLabel* descriptionLabel;
    QHBoxLayout* hbox;
    QPointer<TubeLabel> metadataLabel;
    SubscribeWidget* subscribeWidget;
    QVBoxLayout* textVbox;
    TubeLabel* thumbLabel;
    ChannelLabel* titleLabel;
};
