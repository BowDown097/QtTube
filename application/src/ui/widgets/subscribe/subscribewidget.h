#pragma once
#include "qttube-plugin/objects/channel.h"
#include <QWidget>

class NotificationBell;
struct PluginData;
class QHBoxLayout;
class SubscribeLabel;
class TubeLabel;

class SubscribeWidget : public QWidget
{
public:
    explicit SubscribeWidget(PluginData* plugin, QWidget* parent = nullptr);
    void setData(const QtTubePlugin::SubscribeButton& data);

    QHBoxLayout* layout;
    TubeLabel* subscribersCountLabel;
private:
    NotificationBell* m_notificationBell;
    SubscribeLabel* m_subscribeLabel;
};
