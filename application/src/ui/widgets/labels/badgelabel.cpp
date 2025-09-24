#include "badgelabel.h"

namespace
{
    QString stylesheetFormat = QStringLiteral("QLabel { background: %1; border-radius: 1px; color: %2; }");
}

BadgeLabel::BadgeLabel(QWidget* parent) : QLabel(parent)
{
    setAlignment(Qt::AlignCenter);
    setContentsMargins(3, 1, 3, 1);
    setFont(QFont(font().toString(), 8));
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void BadgeLabel::enterEvent(QEnterEvent*)
#else
void ChannelBadgeLabel::enterEvent(QEvent*)
#endif
{
    setStyleSheet(m_hoveredStylesheet);
}

void BadgeLabel::leaveEvent(QEvent*)
{
    setStyleSheet(m_normalStylesheet);
}

void BadgeLabel::setData(const QtTubePlugin::Badge& badge)
{
    m_hoveredStylesheet = stylesheetFormat.arg(
        badge.colorPalette.hoveredBackground, badge.colorPalette.hoveredForeground);
    m_normalStylesheet = stylesheetFormat.arg(
        badge.colorPalette.background, badge.colorPalette.foreground);

    setStyleSheet(m_normalStylesheet);
    setText(badge.label);
    setToolTip(badge.tooltip);
}
