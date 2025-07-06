#include "notificationbell.h"
#include "qttubeapplication.h"
#include "utils/uiutils.h"
#include <QMenu>

constexpr QLatin1String Stylesheet(R"(
    QToolButton {
        border: 1px solid #555;
        border-radius: 2px;
        padding-left: 2px;
        padding-bottom: 1px;
    }

    QToolButton::menu-indicator {
        image: none;
    }
)");

NotificationBell::NotificationBell(QWidget* parent)
    : QToolButton(parent), m_notificationMenu(new QMenu(this))
{
    setFixedSize(24, 24);
    setStyleSheet(Stylesheet);
    connect(this, &QToolButton::triggered, this, &QToolButton::setDefaultAction);

    setMenu(m_notificationMenu);
    setPopupMode(QToolButton::InstantPopup);
    setToolButtonStyle(Qt::ToolButtonIconOnly);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void NotificationBell::enterEvent(QEnterEvent*)
#else
void NotificationBell::enterEvent(QEvent*)
#endif
{
    setCursor(QCursor(Qt::PointingHandCursor));
}

void NotificationBell::leaveEvent(QEvent*)
{
    setCursor(QCursor());
}

void NotificationBell::setData(const QtTubePlugin::NotificationBell& notificationBell)
{
    m_defaultEnabledStateIndex = notificationBell.defaultEnabledStateIndex;

    for (qsizetype i = 0; i < notificationBell.states.size(); ++i)
    {
        const QtTubePlugin::NotificationState& state = notificationBell.states[i];

        QAction* action = new QAction(state.name, this);
        m_notificationMenu->addAction(action);
        connect(action, &QAction::triggered, this, [this, state] { setState(state); });

        switch (state.representation)
        {
        case QtTubePlugin::NotificationState::Representation::All:
            action->setIcon(UIUtils::iconThemed("notif-bell-all"));
            break;
        case QtTubePlugin::NotificationState::Representation::None:
            action->setIcon(UIUtils::iconThemed("notif-bell-none"));
            break;
        default:
            action->setIcon(UIUtils::iconThemed("notif-bell"));
            break;
        }
    }

    if (notificationBell.activeStateIndex != -1)
        setVisualState(notificationBell.activeStateIndex);
}

void NotificationBell::setVisualState(qsizetype index)
{
    setDefaultAction(m_notificationMenu->actions().at(index));
}

void NotificationBell::setState(const QtTubePlugin::NotificationState& state)
{
    if (const PluginData* activePlugin = qtTubeApp->plugins().activePlugin())
        activePlugin->interface->setNotificationPreference(state.data);
}
