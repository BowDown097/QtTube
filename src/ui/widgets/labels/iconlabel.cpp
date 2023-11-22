#include "iconlabel.h"
#include "utils/uiutils.h"
#include <QBoxLayout>
#include <QMouseEvent>

IconLabel::IconLabel(const QString& iconId, const QMargins& contentsMargins, const QSize& size, QWidget* parent)
    : QWidget(parent), icon(new QLabel(this)), textLabel(new QLabel(this)), layout(new QHBoxLayout(this))
{
    icon->setFixedSize(size);
    icon->setPixmap(UIUtils::pixmapThemed(iconId));
    icon->setScaledContents(true);

    layout->setContentsMargins(contentsMargins);
    layout->addSpacing(2);

    layout->addWidget(icon);
    layout->addWidget(textLabel);
}

IconLabel::IconLabel(const QString& iconId, const QString& text, const QMargins& contentsMargins, const QSize& size, QWidget* parent)
    : IconLabel(iconId, contentsMargins, size, parent)
{
    textLabel->setText(text);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void IconLabel::enterEvent(QEnterEvent*)
#else
void IconLabel::enterEvent(QEvent*)
#endif
{
    setCursor(QCursor(Qt::PointingHandCursor));
}

void IconLabel::leaveEvent(QEvent*)
{
    setCursor(QCursor());
}

void IconLabel::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
        emit clicked();
}

void IconLabel::setIcon(const QString& iconId)
{
    icon->setPixmap(UIUtils::pixmapThemed(iconId));
}
