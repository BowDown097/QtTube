#pragma once
#include "qttube-plugin/objects/channel.h"
#include <QPointer>
#include <QWidget>

class NotificationBell;
class QHBoxLayout;
class SubscribeLabel;
class TubeLabel;

class SubscribeWidget : public QWidget
{
public:
    explicit SubscribeWidget(QWidget* parent = nullptr);
    void setData(const QtTubePlugin::SubscribeButton& data);

    QHBoxLayout* layout;
    QPointer<TubeLabel> subscribersCountLabel;
private:
    NotificationBell* notificationBell;
    SubscribeLabel* subscribeLabel;
};
