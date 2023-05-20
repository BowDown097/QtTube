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
    const QString subscribersCountStyle = R"(
    border: 1px solid #333;
    font-size: 11px;
    line-height: 24px;
    padding: 0 6px 0 4.5px;
    border-radius: 2px;
    text-align: center;
    )";
};

#endif // SUBSCRIBEWIDGET_H
