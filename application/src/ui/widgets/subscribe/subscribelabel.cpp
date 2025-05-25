#include "subscribelabel.h"
#include "innertube.h"
#include "qttubeapplication.h"
#include <QMessageBox>

constexpr QLatin1String SubscribeStylesheet(R"(
    font-size: 12px;
    line-height: 22px;
    border-radius: 2px;
    padding: 0 6px 1px 11px;
)");
constexpr QLatin1String SubscribeHoveredStylesheet(R"(
    font-size: 12px;
    line-height: 22px;
    border-radius: 2px;
    padding: 0 6px 1px 11px;
)");
constexpr QLatin1String SubscribedStylesheet(R"(
    font-size: 12px;
    line-height: 22px;
    border-radius: 2px;
    padding: 0 6px 1px 6px;
)");
constexpr QLatin1String UnsubscribeStylesheet(R"(
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

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void SubscribeLabel::enterEvent(QEnterEvent* event)
#else
void SubscribeLabel::enterEvent(QEvent* event)
#endif
{
    ClickableWidget<QLabel>::enterEvent(event);
    setStyle(subscribed, true);
    if (subscribed)
        setText(localization.unsubscribeText);
}

void SubscribeLabel::leaveEvent(QEvent* event)
{
    ClickableWidget<QLabel>::leaveEvent(event);
    setStyle(subscribed, false);
    if (subscribed)
        setText(localization.subscribedText);
}

void SubscribeLabel::setData(const QtTube::PluginChannel& channel)
{
    colorPalette = channel.subscribeButton.colorPalette;
    localization = channel.subscribeButton.localization;
    subscribed = channel.subscribeButton.subscribed;
    subscribeData = channel.subscribeButton.subscribeData;
    unsubscribeData = channel.subscribeButton.unsubscribeData;

    setStyle(subscribed, false);
    setText(subscribed ? localization.subscribedText : localization.subscribeText);
}

void SubscribeLabel::setStyle(bool subscribed, bool hovered)
{
    QString *background, *border, *foreground;
    QString stylesheet;

    if (subscribed && hovered)
    {
        background = &colorPalette.unsubscribeBackground;
        border = &colorPalette.unsubscribeBorder;
        foreground = &colorPalette.unsubscribeForeground;
        stylesheet = UnsubscribeStylesheet;
    }
    else if (subscribed)
    {
        background = &colorPalette.subscribedBackground;
        border = &colorPalette.subscribedBorder;
        foreground = &colorPalette.subscribedForeground;
        stylesheet = SubscribedStylesheet;
    }
    else if (hovered)
    {
        background = &colorPalette.subscribeHoveredBackground;
        border = &colorPalette.subscribeHoveredBorder;
        foreground = &colorPalette.subscribeHoveredForeground;
        stylesheet = SubscribeHoveredStylesheet;
    }
    else
    {
        background = &colorPalette.subscribeBackground;
        border = &colorPalette.subscribeBorder;
        foreground = &colorPalette.subscribeForeground;
        stylesheet = SubscribeStylesheet;
    }

    if (!background->isEmpty())
        stylesheet += "\nbackground: " + *background + ';';
    if (!border->isEmpty())
        stylesheet += "\nborder: 1px solid " + *border + ';';
    if (!foreground->isEmpty())
        stylesheet += "\ncolor: " + *foreground + ';';

    setStyleSheet(stylesheet);
}

void SubscribeLabel::setSubscribeButton(const InnertubeObjects::Button& button)
{
    localization.subscribeText = button.text.text;
    setStyle(false, false);
    setText(localization.subscribeText);
}

void SubscribeLabel::setSubscribeButton(const InnertubeObjects::ButtonViewModel& button)
{
    localization.subscribeText = button.title;
    setStyle(false, false);
    setText(localization.subscribeText);
}

void SubscribeLabel::setSubscribeButton(const InnertubeObjects::SubscribeButton& subscribeButton)
{
    subscribed = subscribeButton.subscribed;
    subscribeData = subscribeButton.onSubscribeEndpoints[0]["subscribeEndpoint"];
    localization.subscribeText = subscribeButton.unsubscribedButtonText.text;
    localization.subscribedText = subscribeButton.subscribedButtonText.text;
    localization.unsubscribeText = subscribeButton.unsubscribeButtonText.text;

    const QJsonValue unsubscribeDialog = subscribeButton.onUnsubscribeEndpoints
        [0]["signalServiceEndpoint"]["actions"][0]["openPopupAction"]["popup"]["confirmDialogRenderer"];
    localization.unsubscribeDialogText = InnertubeObjects::InnertubeString(unsubscribeDialog["dialogMessages"][0]).text;
    unsubscribeData = unsubscribeDialog["confirmButton"]["buttonRenderer"]["serviceEndpoint"]["unsubscribeEndpoint"];

    setStyle(subscribed, false);
    setText(subscribed ? localization.subscribedText : localization.subscribeText);
}

void SubscribeLabel::setSubscribeButton(
    const InnertubeObjects::SubscribeButtonViewModel& subscribeViewModel, bool subscribed)
{
    this->subscribed = subscribed;
    subscribeData = subscribeViewModel.subscribeButtonContent.onTapCommand["innertubeCommand"]["subscribeEndpoint"];
    localization.subscribeText = subscribeViewModel.subscribeButtonContent.buttonText;
    localization.subscribedText = subscribeViewModel.unsubscribeButtonContent.buttonText;
    localization.unsubscribeText = "Unsubscribe";

    const QJsonValue unsubscribeDialog = subscribeViewModel.unsubscribeButtonContent.onTapCommand["innertubeCommand"]
                                         ["signalServiceEndpoint"]["actions"][0]["openPopupAction"]
                                         ["popup"]["confirmDialogRenderer"];
    localization.unsubscribeDialogText = InnertubeObjects::InnertubeString(unsubscribeDialog["dialogMessages"][0]).text;
    unsubscribeData = unsubscribeDialog["confirmButton"]["buttonRenderer"]["serviceEndpoint"]["unsubscribeEndpoint"];

    setStyle(subscribed, false);
    setText(subscribed ? localization.subscribedText : localization.subscribeText);
}

void SubscribeLabel::toggleSubscriptionStatus(bool subscribed, const QString& newText)
{
    setStyle(subscribed, false);
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

    if (subscribed && QMessageBox::question(nullptr, localization.unsubscribeText, localization.unsubscribeDialogText) == QMessageBox::StandardButton::Yes)
    {
        toggleSubscriptionStatus(false, localization.subscribeText);
        if (const QJsonValue* unsubscribeEndpoint = std::any_cast<QJsonValue>(&unsubscribeData))
            InnerTube::instance()->subscribe(*unsubscribeEndpoint, false);
        else if (const PluginData* activePlugin = qtTubeApp->plugins().activePlugin())
            activePlugin->interface->subscribe(subscribeData);
    }
    else if (!subscribed)
    {
        toggleSubscriptionStatus(true, localization.subscribedText);
        if (const QJsonValue* subscribeEndpoint = std::any_cast<QJsonValue>(&subscribeData))
            InnerTube::instance()->subscribe(*subscribeEndpoint, true);
        else if (const PluginData* activePlugin = qtTubeApp->plugins().activePlugin())
            activePlugin->interface->unsubscribe(unsubscribeData);
    }
}
