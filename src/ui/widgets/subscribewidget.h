#ifndef SUBSCRIBEWIDGET_H
#define SUBSCRIBEWIDGET_H
#include "labels/tubelabel.h"
#include "subscribelabel.h"
#include <QHBoxLayout>

class SubscribeWidget : public QWidget
{
public:
    SubscribeWidget(QWidget* parent = nullptr);
    QHBoxLayout* layout() const { return m_layout; }
    SubscribeLabel* subscribeLabel() const { return m_subscribeLabel; }
    TubeLabel* subscribersCountLabel() const { return m_subscribersCountLabel; }
    void setPreferredPalette(const QPalette& pal);
    void setSubscribeButton(const InnertubeObjects::SubscribeButton& subscribeButton);
    void setSubscriberCount(const QString& subscriberCountText, const QString& channelId);
private:
    QHBoxLayout* m_layout;
    InnertubeObjects::SubscribeButton m_subscribeButton;
    SubscribeLabel* m_subscribeLabel;
    TubeLabel* m_subscribersCountLabel;

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
