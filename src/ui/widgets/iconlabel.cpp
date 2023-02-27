#include "iconlabel.h"
#include <QApplication>
#include <QMouseEvent>

IconLabel::IconLabel(const QString& iconId, const QMargins& contentsMargins, QWidget* parent) : QWidget(parent)
{
    layout = new QHBoxLayout(this);
    layout->setContentsMargins(contentsMargins);
    setLayout(layout);

    bool preferDark = qApp->palette().alternateBase().color().lightness() < 60;

    icon = new QLabel(this);
    icon->setPixmap(QPixmap(QString(preferDark ? ":/%1-light.png" : ":/%1.png").arg(iconId))
                    .scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

    textLabel = new QLabel(this);

    layout->addWidget(icon);
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
