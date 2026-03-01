#include "notificationbell.h"
#include "plugins/pluginentry.h"
#include "utils/uiutils.h"
#include <QMenu>
#include <QMessageBox>

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

NotificationBell::NotificationBell(PluginEntry* plugin, QWidget* parent)
    : QToolButton(parent), m_notificationMenu(new QMenu(this)), m_plugin(plugin)
{
    setFixedSize(24, 24);
    setStyleSheet(Stylesheet);
    connect(this, &QToolButton::triggered, this, &QToolButton::setDefaultAction);

    setMenu(m_notificationMenu);
    setPopupMode(QToolButton::InstantPopup);
    setToolButtonStyle(Qt::ToolButtonIconOnly);
}

void NotificationBell::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::PaletteChange)
    {
        const QList<QAction*> actions = m_notificationMenu->actions();
        for (qsizetype i = 0; i < actions.size(); ++i)
            actions[i]->setIcon(UIUtils::iconThemed(m_icons[i]));
    }

    QWidget::changeEvent(event);
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
    m_icons.clear();

    for (qsizetype i = 0; i < notificationBell.states.size(); ++i)
    {
        const QtTubePlugin::NotificationState& state = notificationBell.states[i];

        QAction* action = new QAction(state.name, this);
        m_notificationMenu->addAction(action);
        connect(action, &QAction::triggered, this, [this, state] { setState(state); });

        QString iconName;
        switch (state.representation)
        {
        case QtTubePlugin::NotificationState::Representation::All:
            iconName = "notif-bell-all";
            break;
        case QtTubePlugin::NotificationState::Representation::None:
            iconName = "notif-bell-none";
            break;
        default:
            iconName = "notif-bell";
            break;
        }

        action->setIcon(UIUtils::iconThemed(iconName));
        m_icons.append(std::move(iconName));
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
    if (!m_plugin->interface->setNotificationPreference(state.data))
        QMessageBox::warning(nullptr, "Feature Not Available", "This feature is not supported by the active plugin.");
}
