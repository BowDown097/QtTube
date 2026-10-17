#include "subscribelabel.hpp"
#include <QMessageBox>
#include <qttube-plugin/providers/providertypes.h>

namespace
{
    const QString subscribeStylesheet = QStringLiteral(R"(
        font-size: 12px;
        line-height: 22px;
        border-radius: 2px;
    )");
}

SubscribeLabel::SubscribeLabel(PluginEntry* plugin, QWidget* parent)
    : ClickableWidget<QLabel>(parent), m_plugin(plugin)
{
    setAlignment(Qt::AlignCenter);
    setFixedHeight(24);
    setStyleSheet(subscribeStylesheet);
    connect(this, &ClickableWidget<QLabel>::clicked, this, &SubscribeLabel::trySubscribe);
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

void SubscribeLabel::setData(const QString& channelId, const QtTubePlugin::SubscribeButton& data)
{
    m_channelId = channelId;
    m_data = data;

    setClickable(m_data.enabled && m_plugin->providers.channelSub != nullptr);
    setText(m_data.subscribed ? m_data.localization.subscribedText : m_data.localization.subscribeText);

    updateWidth();
    setStyle(false);
}

void SubscribeLabel::setStyle(bool hovered)
{
    QString *background, *border, *foreground;
    if (!m_data.enabled)
    {
        background = &m_data.colorPalette.subscribeDisabledBackground;
        border = &m_data.colorPalette.subscribeDisabledBorder;
        foreground = &m_data.colorPalette.subscribeDisabledForeground;
    }
    else if (m_data.subscribed && hovered)
    {
        background = &m_data.colorPalette.unsubscribeBackground;
        border = &m_data.colorPalette.unsubscribeBorder;
        foreground = &m_data.colorPalette.unsubscribeForeground;
    }
    else if (m_data.subscribed)
    {
        background = &m_data.colorPalette.subscribedBackground;
        border = &m_data.colorPalette.subscribedBorder;
        foreground = &m_data.colorPalette.subscribedForeground;
    }
    else if (hovered)
    {
        background = &m_data.colorPalette.subscribeHoveredBackground;
        border = &m_data.colorPalette.subscribeHoveredBorder;
        foreground = &m_data.colorPalette.subscribeHoveredForeground;
    }
    else
    {
        background = &m_data.colorPalette.subscribeBackground;
        border = &m_data.colorPalette.subscribeBorder;
        foreground = &m_data.colorPalette.subscribeForeground;
    }

    QString stylesheet = subscribeStylesheet;
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

void SubscribeLabel::trySubscribe()
{
    if (!m_plugin->authenticated())
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
            m_plugin->providers.channelSub->unsubscribe(m_channelId, m_data.unsubscribeData);
        }
    }
    else
    {
        toggleSubscriptionStatus();
        m_plugin->providers.channelSub->subscribe(m_channelId, m_data.subscribeData);
    }
}

void SubscribeLabel::updateWidth()
{
    const QFontMetrics fm(font());
    const int maxTextWidth = std::max({
        fm.horizontalAdvance(m_data.localization.subscribeText),
        fm.horizontalAdvance(m_data.localization.subscribedText),
        fm.horizontalAdvance(m_data.localization.unsubscribeText)
    });

    constexpr int hpad = 14; // 7px on each side
    setFixedWidth(maxTextWidth + hpad);
}