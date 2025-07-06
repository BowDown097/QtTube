#pragma once
#include "qttube-plugin/objects/channel.h"
#include <QPointer>
#include <QWidget>

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
private slots:
    void navigateChannel();
};
