#include "subscribewidget.h"
#include "notificationbell.h"
#include "qttubeapplication.h"
#include "subscribelabel.h"
#include "ui/widgets/labels/tubelabel.h"
#include <QHBoxLayout>
#include <QtNetwork/QtNetwork>

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

void SubscribeWidget::setSubscriberCount(const QString& subscriberCountText, const QString& channelId)
{
    if (!qtTubeApp->settings().fullSubs)
    {
        subscribersCountLabel->setText(subscriberCountText.left(subscriberCountText.lastIndexOf(" ")));
        subscribersCountLabel->adjustSize();
        return;
    }

    // QNetworkAccessManager needs to be used here due to a bug with the http library
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    manager->setTransferTimeout(2000);

    QNetworkReply* reply = manager->get(QNetworkRequest("https://api.socialcounts.org/youtube-live-subscriber-count/" + channelId));
    connect(reply, &QNetworkReply::finished, this, [this, reply, subscriberCountText] {
        reply->deleteLater();
        reply->manager()->deleteLater();

        if (reply->error() != QNetworkReply::NoError)
        {
            subscribersCountLabel->setText(subscriberCountText.left(subscriberCountText.lastIndexOf(" ")));
            subscribersCountLabel->adjustSize();
        }
        else
        {
            int subs = QJsonDocument::fromJson(reply->readAll())["est_sub"].toInt();
            subscribersCountLabel->setText(QLocale::system().toString(subs));
            subscribersCountLabel->adjustSize();
        }
    });
}
