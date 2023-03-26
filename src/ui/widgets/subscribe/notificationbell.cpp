#include "notificationbell.h"
#include "innertube.h"
#include "ui/uiutilities.h"

NotificationBell::NotificationBell(QWidget* parent) : QToolButton(parent)
{
    setFixedSize(24, 24);
    setStyleSheet(styles);
    connect(this, &QToolButton::triggered, this, &QToolButton::setDefaultAction);

    allAction = new QAction("All", this);
    noneAction = new QAction("None", this);
    personalizedAction = new QAction("Personalized", this);

    connect(allAction, &QAction::triggered, this, std::bind(&NotificationBell::updateNotificationState, this, "NOTIFICATIONS_ACTIVE"));
    connect(noneAction, &QAction::triggered, this, std::bind(&NotificationBell::updateNotificationState, this, "NOTIFICATIONS_OFF"));
    connect(personalizedAction, &QAction::triggered, this, std::bind(&NotificationBell::updateNotificationState, this, "NOTIFICATIONS_NONE"));

    notificationMenu = new QMenu(this);
    notificationMenu->addAction(allAction);
    notificationMenu->addAction(personalizedAction);
    notificationMenu->addAction(noneAction);

    setMenu(notificationMenu);
    setPopupMode(QToolButton::InstantPopup);
    setToolButtonStyle(Qt::ToolButtonIconOnly);
}

void NotificationBell::setNotificationPreferenceButton(const InnertubeObjects::NotificationPreferenceButton& npb)
{
    notificationPreferenceButton = npb;
    setVisualNotificationState(npb.getCurrentState().stateId);
}

void NotificationBell::setPreferredPalette(const QPalette& pal)
{
    setPalette(pal);
    updateIcons();
}

void NotificationBell::setVisualNotificationState(int stateId)
{
    switch (stateId)
    {
    case 0:
        setDefaultAction(noneAction);
        break;
    case 2:
        setDefaultAction(allAction);
        break;
    default:
        setDefaultAction(personalizedAction);
        break;
    }

    updateIcons();
}


void NotificationBell::updateIcons()
{
    allAction->setIcon(QPixmap(UIUtilities::preferDark(palette()) ? ":/notif-bell-all-light.svg" : ":/notif-bell-all.svg"));
    noneAction->setIcon(QPixmap(UIUtilities::preferDark(palette()) ? ":/notif-bell-none-light.svg" : ":/notif-bell-none.svg"));
    personalizedAction->setIcon(QPixmap(UIUtilities::preferDark(palette()) ? ":/notif-bell-light.svg" : ":/notif-bell.svg"));
}

void NotificationBell::updateNotificationState(const QString& iconType)
{
    const InnertubeObjects::MenuServiceItem& msi = notificationPreferenceButton.getService(iconType);
    const QString params = msi.serviceEndpoint["modifyChannelNotificationPreferenceEndpoint"]["params"].toString();
    InnerTube::instance().get<InnertubeEndpoints::ModifyChannelPreference>(params);
}
