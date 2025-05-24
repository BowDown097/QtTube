#include "subscribelabel.h"
#include "innertube.h"
#include "qttubeapplication.h"
#include <QMessageBox>

constexpr QLatin1String SubscribeStylesheet(R"(
    background: red;
    border: solid 1px transparent;
    font-size: 12px;
    line-height: 22px;
    border-radius: 2px;
    padding: 0 6px 1px 11px;
    color: #fefefe;
)");
constexpr QLatin1String SubscribeHoveredStylesheet(R"(
    background: #d90a17;
    border: solid 1px transparent;
    font-size: 12px;
    line-height: 22px;
    border-radius: 2px;
    padding: 0 6px 1px 11px;
    color: #fefefe;
)");
constexpr QLatin1String SubscribedStylesheet(R"(
    border: 1px solid #555;
    font-size: 12px;
    line-height: 22px;
    border-radius: 2px;
    padding: 0 6px 1px 6px;
)");
constexpr QLatin1String UnsubscribeStylesheet(R"(
    border: 1px solid #555;
    font-size: 12px;
    line-height: 22px;
    border-radius: 2px;
    padding: 0 6px 1px 2.5px;
)");

SubscribeLabel::SubscribeLabel(QWidget* parent) : ClickableWidget<QLabel>(parent)
{
    setClickable(true);
    setFixedSize(80, 24);
    connect(this, &ClickableWidget<QLabel>::clicked, this, &SubscribeLabel::trySubscribe);
}

void SubscribeLabel::setData(const QtTube::PluginChannel& channel)
{
    subscribed = channel.subscribeButton.subscribed;
    subscribeData = channel.subscribeButton.subscribeData;
    subscribeText = channel.subscribeButton.subscribeText;
    subscribedText = channel.subscribeButton.subscribedText;
    unsubscribeData = channel.subscribeButton.unsubscribeData;
    unsubscribeDialogText = channel.subscribeButton.unsubscribeDialogText;
    unsubscribeText = channel.subscribeButton.unsubscribeText;

    setStyleSheet(subscribed ? SubscribedStylesheet : SubscribeStylesheet);
    setText(subscribed ? subscribedText : subscribeText);
}

void SubscribeLabel::setSubscribeButton(const InnertubeObjects::Button& button)
{
    subscribeText = button.text.text;
    setStyleSheet(SubscribeStylesheet);
    setText(subscribeText);
}

void SubscribeLabel::setSubscribeButton(const InnertubeObjects::ButtonViewModel& button)
{
    subscribeText = button.title;
    setStyleSheet(SubscribeStylesheet);
    setText(subscribeText);
}

void SubscribeLabel::setSubscribeButton(const InnertubeObjects::SubscribeButton& subscribeButton)
{
    subscribed = subscribeButton.subscribed;
    subscribeData = subscribeButton.onSubscribeEndpoints[0]["subscribeEndpoint"];
    subscribeText = subscribeButton.unsubscribedButtonText.text;
    subscribedText = subscribeButton.subscribedButtonText.text;
    unsubscribeText = subscribeButton.unsubscribeButtonText.text;

    const QJsonValue unsubscribeDialog = subscribeButton.onUnsubscribeEndpoints
        [0]["signalServiceEndpoint"]["actions"][0]["openPopupAction"]["popup"]["confirmDialogRenderer"];
    unsubscribeDialogText = InnertubeObjects::InnertubeString(unsubscribeDialog["dialogMessages"][0]).text;
    unsubscribeData = unsubscribeDialog["confirmButton"]["buttonRenderer"]["serviceEndpoint"]["unsubscribeEndpoint"];

    setStyleSheet(subscribed ? SubscribedStylesheet : SubscribeStylesheet);
    setText(subscribed ? subscribedText : subscribeText);
}

void SubscribeLabel::setSubscribeButton(const InnertubeObjects::SubscribeButtonViewModel& subscribeViewModel,
                                        bool subscribed)
{
    this->subscribed = subscribed;
    subscribeData = subscribeViewModel.subscribeButtonContent.onTapCommand["innertubeCommand"]["subscribeEndpoint"];
    subscribeText = subscribeViewModel.subscribeButtonContent.buttonText;
    subscribedText = subscribeViewModel.unsubscribeButtonContent.buttonText;
    unsubscribeText = "Unsubscribe";

    const QJsonValue unsubscribeDialog = subscribeViewModel.unsubscribeButtonContent.onTapCommand["innertubeCommand"]
                                         ["signalServiceEndpoint"]["actions"][0]["openPopupAction"]
                                         ["popup"]["confirmDialogRenderer"];
    unsubscribeDialogText = InnertubeObjects::InnertubeString(unsubscribeDialog["dialogMessages"][0]).text;
    unsubscribeData = unsubscribeDialog["confirmButton"]["buttonRenderer"]["serviceEndpoint"]["unsubscribeEndpoint"];

    setStyleSheet(subscribed ? SubscribedStylesheet : SubscribeStylesheet);
    setText(subscribed ? subscribedText : subscribeText);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void SubscribeLabel::enterEvent(QEnterEvent* event)
#else
void SubscribeLabel::enterEvent(QEvent* event)
#endif
{
    ClickableWidget<QLabel>::enterEvent(event);
    if (subscribed)
    {
        setStyleSheet(UnsubscribeStylesheet);
        setText(unsubscribeText);
    }
    else
    {
        setStyleSheet(SubscribeHoveredStylesheet);
    }
}

void SubscribeLabel::leaveEvent(QEvent* event)
{
    ClickableWidget<QLabel>::leaveEvent(event);
    if (subscribed)
    {
        setStyleSheet(SubscribedStylesheet);
        setText(subscribedText);
    }
    else
    {
        setStyleSheet(SubscribeStylesheet);
    }
}

void SubscribeLabel::toggleSubscriptionStatus(const QString& styleSheet, const QString& newText)
{
    setStyleSheet(styleSheet);
    setText(newText);
    subscribed = !subscribed;
    emit subscribeStatusChanged(subscribed);
}

void SubscribeLabel::trySubscribe()
{
    if (!InnerTube::instance()->hasAuthenticated())
    {
        QMessageBox::information(nullptr, "Need to log in", "You must be logged in to subscribe to channels.\nLocal subscriptions are planned, but not implemented.");
        return;
    }

    if (!subscribeData.has_value())
    {
        QMessageBox::information(nullptr, "Unable to [un]subscribe", "Subscription functionality is either unavailable or disabled for this button.");
        return;
    }

    if (subscribed && QMessageBox::question(nullptr, unsubscribeText, unsubscribeDialogText) == QMessageBox::StandardButton::Yes)
    {
        toggleSubscriptionStatus(SubscribeStylesheet, subscribeText);
        if (const QJsonValue* unsubscribeEndpoint = std::any_cast<QJsonValue>(&unsubscribeData))
            InnerTube::instance()->subscribe(*unsubscribeEndpoint, false);
        else if (const PluginData* activePlugin = qtTubeApp->plugins().activePlugin())
            activePlugin->interface->subscribe(subscribeData);
    }
    else if (!subscribed)
    {
        toggleSubscriptionStatus(SubscribedStylesheet, subscribedText);
        if (const QJsonValue* subscribeEndpoint = std::any_cast<QJsonValue>(&subscribeData))
            InnerTube::instance()->subscribe(*subscribeEndpoint, true);
        else if (const PluginData* activePlugin = qtTubeApp->plugins().activePlugin())
            activePlugin->interface->unsubscribe(unsubscribeData);
    }
}
