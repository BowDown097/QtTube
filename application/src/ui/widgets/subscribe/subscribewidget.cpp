#include "subscribewidget.h"
#include "notificationbell.h"
#include "subscribelabel.h"
#include "ui/widgets/labels/tubelabel.h"
#include <QBoxLayout>

constexpr QLatin1String SubscribersCountStylesheet(R"(
    border: 1px solid #555;
    font-size: 11px;
    line-height: 24px;
    padding: 0 6px 0 4.5px;
    border-radius: 2px;
    text-align: center;
)");

SubscribeWidget::SubscribeWidget(QWidget* parent)
    : QWidget(parent),
      layout(new QHBoxLayout(this)),
      notificationBell(new NotificationBell(this)),
      subscribeLabel(new SubscribeLabel(this)),
      subscribersCountLabel(new TubeLabel(this))
{
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    subscribeLabel->hide();
    layout->addWidget(subscribeLabel);

    notificationBell->hide();
    layout->addWidget(notificationBell);

    subscribersCountLabel->hide();
    subscribersCountLabel->setFixedHeight(24);
    subscribersCountLabel->setStyleSheet(SubscribersCountStylesheet);
    layout->addWidget(subscribersCountLabel);

    connect(subscribeLabel, &SubscribeLabel::subscribeStatusChanged, this, [this](bool subscribed)
    {
        if (qsizetype defaultState = notificationBell->defaultEnabledStateIndex(); defaultState >= 0)
            notificationBell->setVisualState(defaultState);
        notificationBell->setVisible(subscribed);
    });
}

void SubscribeWidget::setData(const QtTubePlugin::SubscribeButton& data)
{
    subscribeLabel->setData(data);
    subscribeLabel->show();

    if (data.countText.isEmpty())
    {
        layout->removeWidget(subscribersCountLabel);
        subscribersCountLabel->deleteLater();
    }
    else
    {
        subscribersCountLabel->show();
        subscribersCountLabel->setText(data.countText);
        subscribersCountLabel->adjustSize();
    }

    if (!data.notificationBell.states.isEmpty())
    {
        notificationBell->setData(data.notificationBell);
        notificationBell->setVisible(data.subscribed);
    }
}
