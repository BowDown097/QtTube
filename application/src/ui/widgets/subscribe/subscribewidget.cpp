#include "subscribewidget.h"
#include "notificationbell.h"
#include "subscribelabel.h"
#include "ui/widgets/labels/tubelabel.h"
#include "utils/tubeutils.h"
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
        if (originIsInnertube)
            notificationBell->setVisualState(NotificationBell::PreferenceListState::Personalized);
        else if (qsizetype defaultState = notificationBell->defaultEnabledStateIndex(); defaultState >= 0)
            notificationBell->setVisualState(defaultState);
        notificationBell->setVisible(subscribed);
    });
}

void SubscribeWidget::setData(const QtTube::PluginChannel& channel)
{
    originIsInnertube = false;
    subscribeLabel->setData(channel);
    subscribeLabel->show();

    if (channel.subscribeButton.countText.isEmpty())
    {
        layout->removeWidget(subscribersCountLabel);
        subscribersCountLabel->deleteLater();
    }
    else
    {
        subscribersCountLabel->show();
        subscribersCountLabel->setText(channel.subscribeButton.countText);
        subscribersCountLabel->adjustSize();
    }

    notificationBell->setData(channel.subscribeButton.notificationBell);
    notificationBell->setVisible(channel.subscribeButton.subscribed);
}

void SubscribeWidget::setSubscribeButton(const InnertubeObjects::Button& button)
{
    originIsInnertube = true;
    subscribeLabel->setSubscribeButton(button);
    subscribeLabel->show();
}

void SubscribeWidget::setSubscribeButton(const InnertubeObjects::ButtonViewModel& buttonViewModel)
{
    originIsInnertube = true;
    subscribeLabel->setSubscribeButton(buttonViewModel);
    subscribeLabel->show();
}

void SubscribeWidget::setSubscribeButton(const InnertubeObjects::SubscribeButton& subscribeButton)
{
    originIsInnertube = true;
    if (!subscribeButton.enabled)
        return;

    subscribeLabel->setSubscribeButton(subscribeButton);
    subscribeLabel->show();

    if (subscribeButton.notificationPreferenceButton.states.isEmpty())
        return;

    notificationBell->fromNotificationPreferenceButton(subscribeButton.notificationPreferenceButton);
    notificationBell->setVisible(subscribeButton.subscribed);
}

void SubscribeWidget::setSubscribeButton(const InnertubeObjects::SubscribeButtonViewModel& subscribeViewModel,
                                         bool subscribed)
{
    originIsInnertube = true;
    if (subscribeViewModel.disableSubscribeButton)
        return;

    subscribeLabel->setSubscribeButton(subscribeViewModel, subscribed);
    subscribeLabel->show();

    if (subscribeViewModel.disableNotificationBell)
        return;

    notificationBell->fromListViewModel(subscribeViewModel.onShowSubscriptionOptions["innertubeCommand"]
        ["showSheetCommand"]["panelLoadingStrategy"]["inlineContent"]["sheetViewModel"]["content"]["listViewModel"]);
    notificationBell->setVisible(subscribed);
}

void SubscribeWidget::setSubscriberCount(QString subscriberCountText, const QString& channelId)
{
    subscriberCountText.truncate(subscriberCountText.lastIndexOf(' '));
    TubeUtils::getSubCount(channelId, subscriberCountText).then([this](const std::pair<QString, bool>& result) {
        if (subscribersCountLabel)
        {
            subscribersCountLabel->show();
            subscribersCountLabel->setText(result.first);
            subscribersCountLabel->adjustSize();
        }
    });
}
