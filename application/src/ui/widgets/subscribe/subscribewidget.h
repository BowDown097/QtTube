#pragma once
#include "innertube/objects/channel/subscribebutton.h"
#include "innertube/objects/viewmodels/buttonviewmodel.h"
#include "innertube/objects/viewmodels/subscribebuttonviewmodel.h"
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
    void setSubscribeButton(const InnertubeObjects::Button& button);
    void setSubscribeButton(const InnertubeObjects::ButtonViewModel& buttonViewModel);
    void setSubscribeButton(const InnertubeObjects::SubscribeButton& subscribeButton);
    void setSubscribeButton(const InnertubeObjects::SubscribeButtonViewModel& subscribeViewModel, bool subscribed);
    void setSubscriberCount(QString subscriberCountText, const QString& channelId);

    QHBoxLayout* layout;
    QPointer<TubeLabel> subscribersCountLabel;
private:
    NotificationBell* notificationBell;
    SubscribeLabel* subscribeLabel;
};
