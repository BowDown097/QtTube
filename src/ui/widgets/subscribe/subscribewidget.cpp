#include "subscribewidget.h"
#include "http.h"
#include "settingsstore.h"

SubscribeWidget::SubscribeWidget(QWidget* parent)
    : QWidget(parent),
      layout(new QHBoxLayout(this)),
      notificationBell(new NotificationBell(this)),
      subscribeLabel(new SubscribeLabel(this)),
      subscribersCountLabel(new TubeLabel(this))
{
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    layout->addWidget(subscribeLabel);

    notificationBell->setVisible(false);
    layout->addWidget(notificationBell);

    subscribersCountLabel->setFixedHeight(24);
    subscribersCountLabel->setStyleSheet(subscribersCountStyle);
    layout->addWidget(subscribersCountLabel);

    connect(subscribeLabel, &SubscribeLabel::subscribeStatusChanged, this, [this](bool subscribed)
    {
        notificationBell->setVisualNotificationState(3);
        notificationBell->setVisible(subscribed);
    });
}

void SubscribeWidget::setPreferredPalette(const QPalette& pal)
{
    notificationBell->setPreferredPalette(pal);
    subscribeLabel->setPreferredPalette(pal);
    subscribersCountLabel->setPalette(pal);
}

void SubscribeWidget::setSubscribeButton(const InnertubeObjects::SubscribeButton& subscribeButton)
{
    bool showBell = subscribeButton.subscribed && !subscribeButton.notificationPreferenceButton.states.isEmpty();
    subscribeLabel->setSubscribeButton(subscribeButton);
    notificationBell->setVisible(showBell);
    if (showBell)
        notificationBell->setNotificationPreferenceButton(subscribeButton.notificationPreferenceButton);
}

void SubscribeWidget::setSubscriberCount(const QString& subscriberCountText, const QString& channelId)
{
    if (!SettingsStore::instance().fullSubs)
    {
        subscribersCountLabel->setText(subscriberCountText.left(subscriberCountText.lastIndexOf(" ")));
        subscribersCountLabel->adjustSize();
        return;
    }

    Http http;
    http.setReadTimeout(2000);
    http.setMaxRetries(5);

    HttpReply* reply = http.get(QUrl("https://api.socialcounts.org/youtube-live-subscriber-count/" + channelId));
    connect(reply, &HttpReply::error, this, [this, subscriberCountText] {
        subscribersCountLabel->setText(subscriberCountText.left(subscriberCountText.lastIndexOf(" ")));
        subscribersCountLabel->adjustSize();
    });
    connect(reply, &HttpReply::finished, this, [this](const HttpReply& reply) {
        int subs = QJsonDocument::fromJson(reply.body())["est_sub"].toInt();
        subscribersCountLabel->setText(QLocale::system().toString(subs));
        subscribersCountLabel->adjustSize();
    });
}
