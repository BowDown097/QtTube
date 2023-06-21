#ifndef SUBSCRIBEWIDGET_H
#define SUBSCRIBEWIDGET_H
#include "notificationbell.h"
#include "subscribelabel.h"
#include "ui/widgets/labels/tubelabel.h"
#include <QHBoxLayout>

class SubscribeWidget : public QWidget
{
public:
    explicit SubscribeWidget(QWidget* parent = nullptr);
    QHBoxLayout* layout;
    NotificationBell* notificationBell;
    SubscribeLabel* subscribeLabel;
    TubeLabel* subscribersCountLabel;
    void setPreferredPalette(const QPalette& pal);
    void setSubscribeButton(const InnertubeObjects::SubscribeButton& subscribeButton);
    void setSubscriberCount(const QString& subscriberCountText, const QString& channelId);
private:
    InnertubeObjects::SubscribeButton subscribeButton;
};

#endif // SUBSCRIBEWIDGET_H
