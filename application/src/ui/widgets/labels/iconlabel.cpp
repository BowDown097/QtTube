#include "iconlabel.h"
#include "utils/uiutils.h"
#include <QBoxLayout>
#include <QLabel>

IconLabel::IconLabel(
    const QString& iconId, const QMargins& contentsMargins,
    const QSize& size, QWidget* parent)
    : ClickableWidget<>(parent),
      m_icon(new QLabel(this)),
      m_iconId(iconId),
      m_layout(new QHBoxLayout(this)),
      m_text(new QLabel(this))
{
    setClickable(true);
    m_icon->setFixedSize(size);
    m_icon->setPixmap(UIUtils::pixmapThemed(iconId));
    m_icon->setScaledContents(true);

    m_layout->setContentsMargins(contentsMargins);
    m_layout->addWidget(m_icon);
    m_layout->addSpacerItem(new QSpacerItem(2, 0));
    m_layout->addWidget(m_text);
}

IconLabel::IconLabel(
    const QString& iconId, const QString& text, const QMargins& contentsMargins,
    const QSize& size, QWidget* parent)
    : IconLabel(iconId, contentsMargins, size, parent)
{
    m_text->setText(text);
}

void IconLabel::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::PaletteChange)
        m_icon->setPixmap(UIUtils::pixmapThemed(m_iconId));
    QWidget::changeEvent(event);
}

void IconLabel::setIcon(const QString& iconId)
{
    m_iconId = iconId;
    m_icon->setPixmap(UIUtils::pixmapThemed(iconId));
}

void IconLabel::setStyleSheet(const QString& styleSheet)
{
    m_text->setStyleSheet(styleSheet);
}

void IconLabel::setText(const QString& text)
{
    m_text->setText(text);
}

QString IconLabel::styleSheet() const
{
    return m_text->styleSheet();
}

QString IconLabel::text() const
{
    return m_text->text();
}
