#include "topbarbell.h"
#include <QApplication>
#include <QMouseEvent>

TopBarBell::TopBarBell(QWidget* parent) : QWidget(parent), bell(new QLabel(this)), count(new QLabel(this))
{
    setFixedSize(30, 30);
    bell->setFixedSize(30, 30);
    bell->setScaledContents(true);
    count->move(20, 0);
    count->setFont(QFont(qApp->font().toString(), 9));
}

void TopBarBell::updatePixmap(bool hasNotif, bool preferDark)
{
    bell->setPixmap(hasNotif
                        ? QPixmap(preferDark ? ":/notif-bell-hasnotif-light.svg" : ":/notif-bell-hasnotif.svg")
                        : QPixmap(preferDark ? ":/notif-bell-light.svg" : ":/notif-bell.svg"));
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void TopBarBell::enterEvent(QEnterEvent*)
#else
void TopBarBell::enterEvent(QEvent*)
#endif
{
    setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
}

void TopBarBell::leaveEvent(QEvent*)
{
    setCursor(QCursor());
}

void TopBarBell::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
        emit clicked();
}
