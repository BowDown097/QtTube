#include "notificationbell.h"
#include "ui/uiutilities.h"

NotificationBell::NotificationBell(QWidget* parent) : TubeLabel(parent)
{
    setContentsMargins(3, 0, 0, 2);
    setClickable(true, false);
    setFixedSize(24, 24);
    setPixmap(QPixmap(UIUtilities::preferDark() ? ":/notif-bell-light.svg" : ":/notif-bell.svg"));
    setScaledContents(true);
    setStyleSheet("border: 1px solid #333; border-radius: 2px");
    connect(this, &TubeLabel::clicked, this, &NotificationBell::displayNotificationMenu);
}

void NotificationBell::displayNotificationMenu()
{
    qDebug() << "sneed";
}

void NotificationBell::setPreferredPalette(const QPalette& pal)
{
    preferredPalette = pal;
    setPalette(pal);
}
