#pragma once
#include "qttube-plugin/objects/channel.h"
#include <QPointer>
#include <QWidget>

namespace InnertubeObjects { struct Channel; struct MetadataBadge; struct SubscribeButton; }

class ChannelLabel;
class QHBoxLayout;
class QVBoxLayout;
class SubscribeWidget;
class TubeLabel;

class BrowseChannelRenderer : public QWidget
{
    Q_OBJECT
public:
    explicit BrowseChannelRenderer(QWidget* parent = nullptr);
    void setData(const InnertubeObjects::Channel& channel);
    void setData(const QtTube::PluginChannel& channel);
private:
    QString channelId;
    TubeLabel* descriptionLabel;
    QHBoxLayout* hbox;
    QPointer<TubeLabel> metadataLabel;
    SubscribeWidget* subscribeWidget;
    QVBoxLayout* textVbox;
    TubeLabel* thumbLabel;
    ChannelLabel* titleLabel;
private slots:
    void navigateChannel();
};
