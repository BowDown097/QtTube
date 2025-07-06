#include "subscribelabel.h"
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

void SubscribeLabel::setData(const QtTubePlugin::SubscribeButton& data)
{
    colorPalette = data.colorPalette;
    localization = data.localization;
    subscribed = data.subscribed;
    subscribeData = data.subscribeData;
    unsubscribeData = data.unsubscribeData;

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

void SubscribeLabel::toggleSubscriptionStatus()
{
    subscribed = !subscribed;
    setStyle(subscribed, false);
    setText(subscribed ? localization.subscribedText : localization.subscribeText);
    emit subscribeStatusChanged(subscribed);
}

void SubscribeLabel::trySubscribe()
{
    if (const PluginData* activePlugin = qtTubeApp->plugins().activePlugin())
    {
        if (!activePlugin->auth || !activePlugin->auth->activeLogin())
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
            toggleSubscriptionStatus();
            activePlugin->interface->subscribe(subscribeData);
        }
        else if (!subscribed)
        {
            toggleSubscriptionStatus();
            activePlugin->interface->unsubscribe(unsubscribeData);
        }
    }
}
