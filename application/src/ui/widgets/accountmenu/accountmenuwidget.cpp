#include "accountmenuwidget.h"
#include "mainwindow.h"
#include "plugins/pluginentry.h"
#include "ui/views/viewcontroller.h"
#include "ui/widgets/labels/iconlabel.h"
#include "ui/widgets/topbar/topbar.h"
#include "utils/uiutils.h"
#include <QBoxLayout>

AccountMenuWidget::AccountMenuWidget(PluginEntry* plugin, QWidget* parent)
    : QWidget(parent),
      m_accountLayout(new QVBoxLayout),
      m_accountNameLabel(new TubeLabel(this)),
      m_avatar(new TubeLabel(this)),
      m_handleLabel(new TubeLabel(this)),
      m_headerLayout(new QHBoxLayout),
      m_layout(new QVBoxLayout(this)),
      m_signOutLabel(new IconLabel("sign-out", "Sign out", QMargins(), QSize(24, 24), this)),
      m_switchAccountsLabel(new IconLabel("switch-accounts", "Switch account", QMargins(), QSize(24, 24), this)),
      m_yourChannelLabel(new IconLabel("your-channel", "Your channel", QMargins(), QSize(24, 24), this))
{
    if (!plugin->authStore)
        throw std::runtime_error("Account menu somehow opened without auth support.");

    const QtTubePlugin::AuthUser* user = plugin->authStore->activeBaseLogin();
    if (!user)
        throw std::runtime_error("Account menu somehow opened without an active login.");

    setAutoFillBackground(true);

    m_accountNameLabel->setFont(QFont(font().toString(), -1, QFont::Bold));
    m_accountNameLabel->setText(user->username);
    m_accountLayout->addWidget(m_accountNameLabel);

    m_handleLabel->setText(user->handle);
    m_accountLayout->addWidget(m_handleLabel);

    m_avatar->setFixedSize(48, 48);
    m_avatar->setScaledContents(true);
    m_avatar->setImage(user->avatar, TubeLabel::Cached | TubeLabel::Rounded);
    m_headerLayout->addWidget(m_avatar);

    m_headerLayout->addLayout(m_accountLayout);

    m_layout->addLayout(m_headerLayout);
    m_layout->addWidget(m_switchAccountsLabel);
    m_layout->addWidget(m_yourChannelLabel);
    m_layout->addWidget(m_signOutLabel);
    m_layout->addStretch();
    m_layout->setSizeConstraint(QLayout::SetFixedSize);

    connect(m_switchAccountsLabel, &IconLabel::clicked, this, &AccountMenuWidget::accountSwitcherRequested);
    connect(m_signOutLabel, &IconLabel::clicked, this, &AccountMenuWidget::triggerSignOut);
    connect(m_yourChannelLabel, &IconLabel::clicked, this,
            std::bind(&AccountMenuWidget::gotoChannel, this, user->id, plugin));
}

void AccountMenuWidget::gotoChannel(const QString& channelId, PluginEntry* plugin)
{
    hide();
    ViewController::loadChannel(channelId, plugin);
    emit closeRequested();
}

void AccountMenuWidget::triggerSignOut()
{
    hide();
    UIUtils::getMainWindow()->topbar()->signOut();
    emit closeRequested();
}
