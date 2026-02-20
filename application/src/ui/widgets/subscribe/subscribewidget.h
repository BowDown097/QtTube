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
    QHBoxLayout* layout;

    explicit SubscribeWidget(PluginData* plugin, QWidget* parent = nullptr);
    void setData(const QtTubePlugin::SubscribeButton& data);
protected:
    void changeEvent(QEvent* event) override;
private:
    NotificationBell* m_notificationBell;
    SubscribeLabel* m_subscribeLabel;
    TubeLabel* m_subscribersCountLabel;
};
