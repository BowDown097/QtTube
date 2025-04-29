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

    subscribersCountLabel->setFixedHeight(24);
    subscribersCountLabel->setStyleSheet(SubscribersCountStylesheet);
    layout->addWidget(subscribersCountLabel);

    connect(subscribeLabel, &SubscribeLabel::subscribeStatusChanged, this, [this](bool subscribed)
    {
        notificationBell->setVisualNotificationState(NotificationBell::PreferenceListState::Personalized);
        notificationBell->setVisible(subscribed);
    });
}

void SubscribeWidget::setSubscribeButton(const InnertubeObjects::Button& button)
{
    subscribeLabel->setSubscribeButton(button);
    subscribeLabel->show();
}

void SubscribeWidget::setSubscribeButton(const InnertubeObjects::ButtonViewModel& buttonViewModel)
{
    subscribeLabel->setSubscribeButton(buttonViewModel);
    subscribeLabel->show();
}

void SubscribeWidget::setSubscribeButton(const InnertubeObjects::SubscribeButton& subscribeButton)
{
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
            subscribersCountLabel->setText(result.first);
            subscribersCountLabel->adjustSize();
        }
    });
}
