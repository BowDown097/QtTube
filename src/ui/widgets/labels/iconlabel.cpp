#include "iconlabel.h"
#include "ui/uiutilities.h"
#include <QMouseEvent>

IconLabel::IconLabel(const QString& iconId, const QMargins& contentsMargins, QWidget* parent)
    : QWidget(parent), icon(new QLabel(this)), textLabel(new QLabel(this)), layout(new QHBoxLayout(this))
{
    icon->setFixedSize(16, 16);
    icon->setPixmap(QPixmap(QString(UIUtilities::preferDark() ? ":/%1-light.svg" : ":/%1.svg").arg(iconId)));
    icon->setScaledContents(true);
    layout->addWidget(icon);

    layout->setContentsMargins(contentsMargins);
    layout->addSpacing(2);
    layout->addWidget(textLabel);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void IconLabel::enterEvent(QEnterEvent*)
#else
void IconLabel::enterEvent(QEvent*)
#endif
{
    setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
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

void IconLabel::setText(const QString& text)
{
    textLabel->setText(text);
}
