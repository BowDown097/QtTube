#include "iconlabel.h"
#include "utils/uiutils.h"
#include <QBoxLayout>
#include <QMouseEvent>

IconLabel::IconLabel(const QString& iconId, const QMargins& contentsMargins, const QSize& size, QWidget* parent)
    : ClickableWidget<QWidget>(parent), icon(new QLabel(this)), textLabel(new QLabel(this)), layout(new QHBoxLayout(this))
{
    setClickable(true);
    icon->setFixedSize(size);
    icon->setPixmap(UIUtils::pixmapThemed(iconId));
    icon->setScaledContents(true);

    layout->setContentsMargins(contentsMargins);

    layout->addWidget(icon);
    layout->addSpacerItem(new QSpacerItem(2, 0));
    layout->addWidget(textLabel);
}

IconLabel::IconLabel(const QString& iconId, const QString& text, const QMargins& contentsMargins,
                     const QSize& size, QWidget* parent)
    : IconLabel(iconId, contentsMargins, size, parent)
{
    textLabel->setText(text);
}

void IconLabel::setIcon(const QString& iconId)
{
    icon->setPixmap(UIUtils::pixmapThemed(iconId));
}
