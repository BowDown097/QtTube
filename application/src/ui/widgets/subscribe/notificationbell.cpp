#include "notificationbell.h"
#include "innertube.h"
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

void NotificationBell::addInnertubeStates()
{
    QAction* allAction = new QAction("All", this);
    allAction->setIcon(UIUtils::iconThemed("notif-bell-all"));
    m_notificationMenu->addAction(allAction);
    connect(allAction, &QAction::triggered, this, [this] { setState(PreferenceListState::All); });

    QAction* personalizedAction = new QAction("Personalized", this);
    personalizedAction->setIcon(UIUtils::iconThemed("notif-bell"));
    m_notificationMenu->addAction(personalizedAction);
    connect(personalizedAction, &QAction::triggered, this, [this] { setState(PreferenceListState::Personalized); });

    QAction* noneAction = new QAction("None", this);
    noneAction->setIcon(UIUtils::iconThemed("notif-bell-none"));
    m_notificationMenu->addAction(noneAction);
    connect(noneAction, &QAction::triggered, this, [this] { setState(PreferenceListState::None); });
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

// this method to get the current state is pretty voodoo, i don't like it.
// i couldn't find one that wouldn't require even more voodoo unfortunately :(
void NotificationBell::fromListViewModel(const QJsonValue& listViewModel)
{
    addInnertubeStates();

    constexpr qsizetype MaxStateValue = 2;
    PreferenceListState currentState = PreferenceListState::Personalized;
    const QJsonArray listItems = listViewModel["listItems"].toArray();

    for (qsizetype i = 0; i < std::min(listItems.size(), MaxStateValue + 1); ++i)
    {
        const QJsonValue viewModel = listItems[i]["listItemViewModel"];
        if (viewModel["isSelected"].toBool())
            currentState = static_cast<PreferenceListState>(i);
        m_serviceParams.append(viewModel["rendererContext"]["commandContext"]["onTap"]["innertubeCommand"]
                                      ["modifyChannelNotificationPreferenceEndpoint"]["params"].toString());
    }

    setVisualState(currentState);
}

void NotificationBell::fromNotificationPreferenceButton(const InnertubeObjects::NotificationPreferenceButton& npb)
{
    addInnertubeStates();
    if (const InnertubeObjects::NotificationState* currentState = npb.getCurrentState())
        setVisualState(static_cast<PreferenceButtonState>(currentState->stateId));
    for (const InnertubeObjects::MenuServiceItem& msi : npb.popup.items)
        m_serviceParams.append(msi.serviceEndpoint["modifyChannelNotificationPreferenceEndpoint"]["params"].toString());
}

void NotificationBell::setData(const QtTube::PluginNotificationBell& notificationBell)
{
    m_defaultEnabledStateIndex = notificationBell.defaultEnabledStateIndex;

    for (qsizetype i = 0; i < notificationBell.states.size(); ++i)
    {
        const QtTube::PluginNotificationState& state = notificationBell.states[i];

        QAction* action = new QAction(state.name, this);
        m_notificationMenu->addAction(action);
        connect(action, &QAction::triggered, this, [this, state] { setState(state); });

        switch (state.representation)
        {
        case QtTube::PluginNotificationState::Representation::All:
            action->setIcon(UIUtils::iconThemed("notif-bell-all"));
            break;
        case QtTube::PluginNotificationState::Representation::None:
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

void NotificationBell::setVisualState(PreferenceButtonState state)
{
    switch (state)
    {
    case PreferenceButtonState::None:
        setDefaultAction(m_notificationMenu->actions().at(2));
        break;
    case PreferenceButtonState::All:
        setDefaultAction(m_notificationMenu->actions().at(0));
        break;
    case PreferenceButtonState::Personalized:
        setDefaultAction(m_notificationMenu->actions().at(1));
        break;
    }
}

void NotificationBell::setVisualState(PreferenceListState state)
{
    switch (state)
    {
    case PreferenceListState::None:
        setDefaultAction(m_notificationMenu->actions().at(2));
        break;
    case PreferenceListState::All:
        setDefaultAction(m_notificationMenu->actions().at(0));
        break;
    case PreferenceListState::Personalized:
        setDefaultAction(m_notificationMenu->actions().at(1));
        break;
    }
}

void NotificationBell::setState(PreferenceListState state)
{
    InnerTube::instance()->get<InnertubeEndpoints::ModifyChannelPreference>(m_serviceParams[static_cast<int>(state)]);
}

void NotificationBell::setState(const QtTube::PluginNotificationState& state)
{
    if (const PluginData* activePlugin = qtTubeApp->plugins().activePlugin())
        activePlugin->interface->setNotificationPreference(state.data);
}
