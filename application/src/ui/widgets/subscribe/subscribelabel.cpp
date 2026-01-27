#include "subscribelabel.h"
#include "plugins/pluginmanager.h"
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

SubscribeLabel::SubscribeLabel(PluginData* plugin, QWidget* parent)
    : ClickableWidget<QLabel>(parent)
{
    setFixedSize(80, 24);
    connect(this, &ClickableWidget<QLabel>::clicked, this,
            std::bind(&SubscribeLabel::trySubscribe, this, plugin));
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void SubscribeLabel::enterEvent(QEnterEvent* event)
#else
void SubscribeLabel::enterEvent(QEvent* event)
#endif
{
    setStyle(true);
    if (subscribed)
        setText(localization.unsubscribeText);
    ClickableWidget<QLabel>::enterEvent(event);
}

void SubscribeLabel::leaveEvent(QEvent* event)
{
    setStyle(false);
    if (subscribed)
        setText(localization.subscribedText);
    ClickableWidget<QLabel>::leaveEvent(event);
}

void SubscribeLabel::setData(const QtTubePlugin::SubscribeButton& data)
{
    colorPalette = data.colorPalette;
    enabled = data.enabled;
    localization = data.localization;
    subscribed = data.subscribed;
    subscribeData = data.subscribeData;
    unsubscribeData = data.unsubscribeData;

    setClickable(enabled);
    setStyle(false);
    setText(subscribed ? localization.subscribedText : localization.subscribeText);
}

void SubscribeLabel::setStyle(bool hovered)
{
    QString *background, *border, *foreground;
    QString stylesheet;

    if (!enabled)
    {
        background = &colorPalette.subscribeDisabledBackground;
        border = &colorPalette.subscribeDisabledBorder;
        foreground = &colorPalette.subscribeDisabledForeground;
        stylesheet = SubscribeStylesheet;
    }
    else if (subscribed && hovered)
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
    setStyle(false);
    setText(subscribed ? localization.subscribedText : localization.subscribeText);
    emit subscribeStatusChanged(subscribed);
}

void SubscribeLabel::trySubscribe(PluginData* plugin)
{
    if (!plugin->auth || plugin->auth->isEmpty())
    {
        QMessageBox::warning(nullptr, "Login Required", "Local subscriptions are not yet available. You will need to log in.");
        return;
    }

    if (!subscribeData.has_value())
    {
        QMessageBox::critical(nullptr, "Failed to Subscribe", "Required data is missing or unavailable.");
        return;
    }

    if (subscribed && QMessageBox::question(nullptr, localization.unsubscribeText, localization.unsubscribeDialogText) == QMessageBox::StandardButton::Yes)
    {
        toggleSubscriptionStatus();
        if (!plugin->interface->unsubscribe(unsubscribeData))
            QMessageBox::warning(nullptr, "Feature Not Available", "This feature is not supported by the active plugin.");
    }
    else if (!subscribed)
    {
        toggleSubscriptionStatus();
        if (!plugin->interface->subscribe(subscribeData))
            QMessageBox::warning(nullptr, "Feature Not Available", "This feature is not supported by the active plugin.");
    }
}
