#include "topbarbell.h"
#include "utils/uiutils.h"
#include <QLabel>

TopBarBell::TopBarBell(QWidget* parent)
    : ClickableWidget<>(parent), m_bell(new QLabel(this)), m_count(new QLabel(this))
{
    setClickable(true);
    setFixedSize(30, 30);

    m_bell->setFixedSize(30, 30);
    m_bell->setPixmap(UIUtils::pixmapThemed("notif-bell"));
    m_bell->setScaledContents(true);
}

QString TopBarBell::countText() const
{
    return m_count->text();
}

bool TopBarBell::hasNotifications() const
{
    return m_count->isVisible();
}

void TopBarBell::updateCount(int unseenCount)
{
    m_count->setText(QString::number(unseenCount));
    m_count->setVisible(unseenCount > 0);

    if (unseenCount > 9)
    {
        m_count->move(18, 0);
        m_count->setFont(QFont(font().toString(), 8));
    }
    else
    {
        m_count->move(20, 0);
        m_count->setFont(QFont(font().toString(), 9));
    }

    m_count->adjustSize();
}

void TopBarBell::updatePixmap(bool hasNotif)
{
    m_bell->setPixmap(UIUtils::pixmapThemed(hasNotif ? "notif-bell-hasnotif" : "notif-bell"));
}
