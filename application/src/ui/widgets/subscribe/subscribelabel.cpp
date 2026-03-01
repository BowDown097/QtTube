#include "subscribelabel.h"
#include "plugins/pluginentry.h"
#include <QMessageBox>

namespace
{
    const QString subscribeStylesheet = QStringLiteral(R"(
        font-size: 12px;
        line-height: 22px;
        border-radius: 2px;
        padding: 0 6px 1px 11px;
    )");
    const QString subscribeHoveredStylesheet = QStringLiteral(R"(
        font-size: 12px;
        line-height: 22px;
        border-radius: 2px;
        padding: 0 6px 1px 11px;
    )");
    const QString subscribedStylesheet = QStringLiteral(R"(
        font-size: 12px;
        line-height: 22px;
        border-radius: 2px;
        padding: 0 6px 1px 6px;
    )");
    const QString unsubscribeStylesheet = QStringLiteral(R"(
        font-size: 12px;
        line-height: 22px;
        border-radius: 2px;
        padding: 0 6px 1px 2.5px;
    )");
}

SubscribeLabel::SubscribeLabel(PluginEntry* plugin, QWidget* parent)
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
    if (m_data.subscribed)
        setText(m_data.localization.unsubscribeText);
    ClickableWidget<QLabel>::enterEvent(event);
}

void SubscribeLabel::leaveEvent(QEvent* event)
{
    setStyle(false);
    if (m_data.subscribed)
        setText(m_data.localization.subscribedText);
    ClickableWidget<QLabel>::leaveEvent(event);
}

void SubscribeLabel::setData(const QtTubePlugin::SubscribeButton& data)
{
    m_data = data;
    setClickable(m_data.enabled);
    setStyle(false);
    setText(m_data.subscribed ? m_data.localization.subscribedText : m_data.localization.subscribeText);
}

void SubscribeLabel::setStyle(bool hovered)
{
    QString *background, *border, *foreground;
    QString stylesheet;

    if (!m_data.enabled)
    {
        background = &m_data.colorPalette.subscribeDisabledBackground;
        border = &m_data.colorPalette.subscribeDisabledBorder;
        foreground = &m_data.colorPalette.subscribeDisabledForeground;
        stylesheet = subscribeStylesheet;
    }
    else if (m_data.subscribed && hovered)
    {
        background = &m_data.colorPalette.unsubscribeBackground;
        border = &m_data.colorPalette.unsubscribeBorder;
        foreground = &m_data.colorPalette.unsubscribeForeground;
        stylesheet = unsubscribeStylesheet;
    }
    else if (m_data.subscribed)
    {
        background = &m_data.colorPalette.subscribedBackground;
        border = &m_data.colorPalette.subscribedBorder;
        foreground = &m_data.colorPalette.subscribedForeground;
        stylesheet = subscribedStylesheet;
    }
    else if (hovered)
    {
        background = &m_data.colorPalette.subscribeHoveredBackground;
        border = &m_data.colorPalette.subscribeHoveredBorder;
        foreground = &m_data.colorPalette.subscribeHoveredForeground;
        stylesheet = subscribeHoveredStylesheet;
    }
    else
    {
        background = &m_data.colorPalette.subscribeBackground;
        border = &m_data.colorPalette.subscribeBorder;
        foreground = &m_data.colorPalette.subscribeForeground;
        stylesheet = subscribeStylesheet;
    }

    if (!background->isEmpty())
        stylesheet += "\nbackground: " % *background % ';';
    if (!border->isEmpty())
        stylesheet += "\nborder: 1px solid " % *border % ';';
    if (!foreground->isEmpty())
        stylesheet += "\ncolor: " % *foreground % ';';

    setStyleSheet(stylesheet);
}

void SubscribeLabel::toggleSubscriptionStatus()
{
    m_data.subscribed = !m_data.subscribed;
    setStyle(false);
    setText(m_data.subscribed ? m_data.localization.subscribedText : m_data.localization.subscribeText);
    emit subscribeStatusChanged(m_data.subscribed);
}

void SubscribeLabel::trySubscribe(PluginEntry* plugin)
{
    if (!plugin->authStore || plugin->authStore->isEmpty())
    {
        QMessageBox::warning(nullptr, "Login Required", "Local subscriptions are not yet available. You will need to log in.");
        return;
    }

    if (!m_data.subscribeData.has_value())
    {
        QMessageBox::critical(nullptr, "Failed to Subscribe", "Required data is missing or unavailable.");
        return;
    }

    if (m_data.subscribed)
    {
        QMessageBox::StandardButton response = QMessageBox::question(
            nullptr, m_data.localization.unsubscribeText, m_data.localization.unsubscribeDialogText);
        if (response == QMessageBox::StandardButton::Yes)
        {
            toggleSubscriptionStatus();
            if (!plugin->interface->unsubscribe(m_data.unsubscribeData))
                QMessageBox::warning(nullptr, "Feature Not Available", "This feature is not supported by the active plugin.");
        }
    }
    else
    {
        toggleSubscriptionStatus();
        if (!plugin->interface->subscribe(m_data.subscribeData))
            QMessageBox::warning(nullptr, "Feature Not Available", "This feature is not supported by the active plugin.");
    }
}
