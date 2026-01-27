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

SubscribeWidget::SubscribeWidget(PluginData* plugin, QWidget* parent)
    : QWidget(parent),
      layout(new QHBoxLayout(this)),
      notificationBell(new NotificationBell(plugin, this)),
      subscribeLabel(new SubscribeLabel(plugin, this)),
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

    if (!data.countText.isEmpty())
    {
        subscribersCountLabel->show();
        subscribersCountLabel->setText(data.countText);
        subscribersCountLabel->adjustSize();
    }
    else
    {
        subscribersCountLabel->hide();
    }

    if (!data.notificationBell.states.isEmpty())
    {
        notificationBell->setData(data.notificationBell);
        notificationBell->setVisible(data.subscribed);
    }
    else
    {
        notificationBell->hide();
    }
}
