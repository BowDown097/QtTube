#include "subscribewidget.h"
#include "innertube.h"
#include <QMessageBox>

SubscribeWidget::SubscribeWidget(const InnertubeObjects::SubscribeButton& subscribeButton, QWidget* parent, Qt::WindowFlags f)
    : QLabel(parent, f), subscribeButton(subscribeButton)
{
    setFixedSize(80, 24);
    setStyleSheet(subscribeButton.subscribed ? subscribedStyle : subscribeStyle);
    setText(subscribeButton.buttonText.text);
}

void SubscribeWidget::enterEvent(QEnterEvent*)
{
    setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
    if (subscribeButton.subscribed)
    {
        setStyleSheet(unsubscribeStyle);
        setText(subscribeButton.unsubscribeButtonText.text);
    }
    else
    {
        setStyleSheet(subscribeHoveredStyle);
    }
}

void SubscribeWidget::leaveEvent(QEvent*)
{
    setCursor(QCursor());
    if (subscribeButton.subscribed)
    {
        setStyleSheet(subscribedStyle);
        setText(subscribeButton.subscribedButtonText.text);
    }
    else
    {
        setStyleSheet(subscribeStyle);
    }
}

void SubscribeWidget::mousePressEvent(QMouseEvent*)
{
    if (!InnerTube::instance().hasAuthenticated())
    {
        QMessageBox::information(nullptr, "Need to log in", "You must be logged in to subscribe to channels.\nLocal subscriptions are planned, but not implemented.");
        return;
    }

    if (subscribeButton.subscribed)
    {
        if (QMessageBox::question(nullptr, "Unsubscribe?", "Unsubscribe from this channel?") != QMessageBox::StandardButton::Yes)
            return;
        InnerTube::instance().subscribe(subscribeButton.onUnsubscribeEndpoints[0], false);
        setStyleSheet(subscribeStyle);
        setText(subscribeButton.unsubscribedButtonText.text);
        subscribeButton.subscribed = false;
    }
    else
    {
        InnerTube::instance().subscribe(subscribeButton.onSubscribeEndpoints[0], true);
        setStyleSheet(subscribedStyle);
        setText(subscribeButton.subscribedButtonText.text);
        subscribeButton.subscribed = true;
    }
}
