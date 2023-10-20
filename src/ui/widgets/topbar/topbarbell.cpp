#include "topbarbell.h"
#include "utils/uiutils.h"
#include <QApplication>
#include <QMouseEvent>

TopBarBell::TopBarBell(QWidget* parent) : QWidget(parent), bell(new QLabel(this)), count(new QLabel(this))
{
    setFixedSize(30, 30);
    bell->setFixedSize(30, 30);
    bell->setScaledContents(true);
}

void TopBarBell::updateCount(int unseenCount)
{
    count->setText(QString::number(unseenCount));
    count->setVisible(unseenCount > 0);

    if (unseenCount > 9)
    {
        count->move(18, 0);
        count->setFont(QFont(qApp->font().toString(), 8));
    }
    else
    {
        count->move(20, 0);
        count->setFont(QFont(qApp->font().toString(), 9));
    }

    count->adjustSize();
}

void TopBarBell::updatePixmap(bool hasNotif, const QPalette& pal)
{
    bell->setPixmap(UIUtils::pixmapThemed(hasNotif ? "notif-bell-hasnotif" : "notif-bell", false, QSize(), pal));
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void TopBarBell::enterEvent(QEnterEvent*)
#else
void TopBarBell::enterEvent(QEvent*)
#endif
{
    setCursor(QCursor(Qt::PointingHandCursor));
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
