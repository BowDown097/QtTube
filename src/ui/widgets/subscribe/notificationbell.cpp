#include "notificationbell.h"
#include "innertube.h"
#include "ui/uiutilities.h"

constexpr const char* stylesheet = R"(
    QToolButton {
        border: 1px solid #555;
        border-radius: 2px;
        padding-left: 2px;
        padding-bottom: 1px;
    }

    QToolButton::menu-indicator {
        image: none;
    }
)";

NotificationBell::NotificationBell(QWidget* parent)
    : QToolButton(parent),
      allAction(new QAction("All", this)),
      noneAction(new QAction("None", this)),
      notificationMenu(new QMenu(this)),
      personalizedAction(new QAction("Personalized", this))
{
    setFixedSize(24, 24);
    setStyleSheet(stylesheet);
    connect(this, &QToolButton::triggered, this, &QToolButton::setDefaultAction);

    connect(allAction, &QAction::triggered, this, std::bind(&NotificationBell::updateNotificationState, this, "NOTIFICATIONS_ACTIVE"));
    connect(noneAction, &QAction::triggered, this, std::bind(&NotificationBell::updateNotificationState, this, "NOTIFICATIONS_OFF"));
    connect(personalizedAction, &QAction::triggered, this, std::bind(&NotificationBell::updateNotificationState, this, "NOTIFICATIONS_NONE"));

    notificationMenu->addAction(allAction);
    notificationMenu->addAction(personalizedAction);
    notificationMenu->addAction(noneAction);

    setMenu(notificationMenu);
    setPopupMode(QToolButton::InstantPopup);
    setToolButtonStyle(Qt::ToolButtonIconOnly);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void NotificationBell::enterEvent(QEnterEvent*)
#else
void NotificationBell::enterEvent(QEvent*)
#endif
{
    setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
}

void NotificationBell::leaveEvent(QEvent*)
{
    setCursor(QCursor());
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
    allAction->setIcon(UIUtilities::iconThemed("notif-bell-all", palette()));
    noneAction->setIcon(UIUtilities::iconThemed("notif-bell-none", palette()));
    personalizedAction->setIcon(UIUtilities::iconThemed("notif-bell", palette()));
}

void NotificationBell::updateNotificationState(const QString& iconType)
{
    const InnertubeObjects::MenuServiceItem& msi = notificationPreferenceButton.getService(iconType);
    const QString params = msi.serviceEndpoint["modifyChannelNotificationPreferenceEndpoint"]["params"].toString();
    InnerTube::instance().get<InnertubeEndpoints::ModifyChannelPreference>(params);
}
