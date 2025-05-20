#include "channelbadgelabel.h"
#include "innertube/objects/channel/metadatabadge.h"

constexpr QLatin1String HoveredStylesheet("QLabel { background: #4aa1df; border-radius: 1px; color: #ddd }");
constexpr QLatin1String NormalStylesheet("QLabel { background: #777; border-radius: 1px; color: #ddd }");

ChannelBadgeLabel::ChannelBadgeLabel(QWidget* parent) : QLabel(parent)
{
    setAlignment(Qt::AlignCenter);
    setFixedSize(13, 10);
    setFont(QFont(font().toString(), 8));
    setStyleSheet(NormalStylesheet);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void ChannelBadgeLabel::enterEvent(QEnterEvent*)
#else
void ChannelBadgeLabel::enterEvent(QEvent*)
#endif
{
    setStyleSheet(HoveredStylesheet);
}

void ChannelBadgeLabel::leaveEvent(QEvent*)
{
    setStyleSheet(NormalStylesheet);
}

void ChannelBadgeLabel::setData(const InnertubeObjects::MetadataBadge& badge)
{
    setText(badge.style == "BADGE_STYLE_TYPE_VERIFIED_ARTIST" ? "♪" : "✔");
    setToolTip(badge.tooltip);
}

void ChannelBadgeLabel::setData(const QtTube::PluginBadge& badge)
{
    setText(badge.label);
    setToolTip(badge.tooltip);
}
