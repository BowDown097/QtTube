#include "topbarbell.h"
#include "utils/uiutils.h"
#include <QApplication>

TopBarBell::TopBarBell(QWidget* parent)
    : ClickableWidget<QWidget>(true, false, parent), bell(new QLabel(this)), count(new QLabel(this))
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
