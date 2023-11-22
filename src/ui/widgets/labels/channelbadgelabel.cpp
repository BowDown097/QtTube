#include "channelbadgelabel.h"
#include "innertube/objects/channel/metadatabadge.h"
#include <QApplication>

constexpr const char* hoveredStylesheet = "QLabel { background: #4aa1df; border-radius: 1px; color: #ddd }";
constexpr const char* normalStylesheet = "QLabel { background: #777; border-radius: 1px; color: #ddd }";

ChannelBadgeLabel::ChannelBadgeLabel(QWidget* parent) : QLabel(parent)
{
    setAlignment(Qt::AlignCenter);
    setFixedSize(13, 10);
    setFont(QFont(qApp->font().toString(), 8));
    setStyleSheet(normalStylesheet);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void ChannelBadgeLabel::enterEvent(QEnterEvent*)
#else
void ChannelBadgeLabel::enterEvent(QEvent*)
#endif
{
    setStyleSheet(hoveredStylesheet);
}

void ChannelBadgeLabel::leaveEvent(QEvent*)
{
    setStyleSheet(normalStylesheet);
}

void ChannelBadgeLabel::setData(const InnertubeObjects::MetadataBadge& badge)
{
    setText(badge.style == "BADGE_STYLE_TYPE_VERIFIED_ARTIST" ? "♪" : "✔");
    setToolTip(badge.tooltip);
}
