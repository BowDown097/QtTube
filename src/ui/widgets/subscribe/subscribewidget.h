#ifndef SUBSCRIBEWIDGET_H
#define SUBSCRIBEWIDGET_H
#include "innertube/objects/channel/subscribebutton.h"
#include <QWidget>

class NotificationBell;
class QHBoxLayout;
class SubscribeLabel;
class TubeLabel;

class SubscribeWidget : public QWidget
{
public:
    explicit SubscribeWidget(QWidget* parent = nullptr);
    void setSubscribeButton(const InnertubeObjects::SubscribeButton& subscribeButton);
    void setSubscriberCount(const QString& subscriberCountText, const QString& channelId);

    QHBoxLayout* layout;
    TubeLabel* subscribersCountLabel;
private:
    NotificationBell* notificationBell;
    InnertubeObjects::SubscribeButton subscribeButton;
    SubscribeLabel* subscribeLabel;
};

#endif // SUBSCRIBEWIDGET_H
