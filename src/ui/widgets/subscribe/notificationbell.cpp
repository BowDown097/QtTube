#include "notificationbell.h"
#include "innertube.h"
#include "utils/uiutils.h"
#include <QMenu>

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

    connect(allAction, &QAction::triggered, this,
            std::bind(&NotificationBell::updateNotificationState, this, PreferenceListState::All));
    connect(noneAction, &QAction::triggered, this,
            std::bind(&NotificationBell::updateNotificationState, this, PreferenceListState::None));
    connect(personalizedAction, &QAction::triggered, this,
            std::bind(&NotificationBell::updateNotificationState, this, PreferenceListState::Personalized));

    notificationMenu->addAction(allAction);
    notificationMenu->addAction(personalizedAction);
    notificationMenu->addAction(noneAction);

    setMenu(notificationMenu);
    setPopupMode(QToolButton::InstantPopup);
    setToolButtonStyle(Qt::ToolButtonIconOnly);

    allAction->setIcon(UIUtils::iconThemed("notif-bell-all"));
    noneAction->setIcon(UIUtils::iconThemed("notif-bell-none"));
    personalizedAction->setIcon(UIUtils::iconThemed("notif-bell"));
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
    int currentState{};
    const QJsonArray listItems = listViewModel["listItems"].toArray();
    for (int i = 0; i < listItems.size(); i++)
    {
        const QJsonValue viewModel = listItems[i]["listItemViewModel"];
        if (viewModel["isSelected"].toBool())
            currentState = i;
        serviceParams.append(viewModel["rendererContext"]["commandContext"]["onTap"]["innertubeCommand"]
                                      ["modifyChannelNotificationPreferenceEndpoint"]["params"].toString());
    }

    setVisualNotificationState(static_cast<PreferenceListState>(currentState));
}

void NotificationBell::fromNotificationPreferenceButton(const InnertubeObjects::NotificationPreferenceButton& npb)
{
    setVisualNotificationState(static_cast<PreferenceButtonState>(npb.getCurrentState().stateId));
    for (const InnertubeObjects::MenuServiceItem& msi : npb.popup.items)
        serviceParams.append(msi.serviceEndpoint["modifyChannelNotificationPreferenceEndpoint"]["params"].toString());
}

void NotificationBell::setVisualNotificationState(PreferenceButtonState state)
{
    switch (state)
    {
    case PreferenceButtonState::None:
        setDefaultAction(noneAction);
        break;
    case PreferenceButtonState::All:
        setDefaultAction(allAction);
        break;
    case PreferenceButtonState::Personalized:
        setDefaultAction(personalizedAction);
        break;
    }
}

void NotificationBell::setVisualNotificationState(PreferenceListState state)
{
    switch (state)
    {
    case PreferenceListState::None:
        setDefaultAction(noneAction);
        break;
    case PreferenceListState::All:
        setDefaultAction(allAction);
        break;
    case PreferenceListState::Personalized:
        setDefaultAction(personalizedAction);
        break;
    }
}

void NotificationBell::updateNotificationState(PreferenceListState state)
{
    InnerTube::instance()->get<InnertubeEndpoints::ModifyChannelPreference>(serviceParams[static_cast<int>(state)]);
}
