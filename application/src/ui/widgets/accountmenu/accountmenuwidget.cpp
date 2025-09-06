#include "accountmenuwidget.h"
#include "mainwindow.h"
#include "plugins/pluginmanager.h"
#include "ui/views/viewcontroller.h"
#include "ui/widgets/labels/iconlabel.h"
#include <QBoxLayout>

AccountMenuWidget::AccountMenuWidget(PluginData* plugin, QWidget* parent)
    : QWidget(parent),
      accountLayout(new QVBoxLayout),
      accountNameLabel(new TubeLabel(this)),
      avatar(new TubeLabel(this)),
      handleLabel(new TubeLabel(this)),
      headerLayout(new QHBoxLayout),
      layout(new QVBoxLayout(this)),
      signOutLabel(new IconLabel("sign-out", "Sign out", QMargins(), QSize(24, 24), this)),
      switchAccountsLabel(new IconLabel("switch-accounts", "Switch account", QMargins(), QSize(24, 24), this)),
      yourChannelLabel(new IconLabel("your-channel", "Your channel", QMargins(), QSize(24, 24), this))
{
    if (!plugin->auth)
        throw std::runtime_error("Account menu somehow opened without auth support.");

    const QtTubePlugin::AuthUser* user = plugin->auth->activeBaseLogin();
    if (!user)
        throw std::runtime_error("Account menu somehow opened without an active login.");

    setAutoFillBackground(true);

    accountNameLabel->setFont(QFont(font().toString(), -1, QFont::Bold));
    accountNameLabel->setText(user->username);
    accountLayout->addWidget(accountNameLabel);

    handleLabel->setText(user->handle);
    accountLayout->addWidget(handleLabel);

    avatar->setFixedSize(48, 48);
    avatar->setScaledContents(true);
    avatar->setImage(user->avatar, TubeLabel::Cached | TubeLabel::Rounded);
    headerLayout->addWidget(avatar);

    headerLayout->addLayout(accountLayout);

    layout->addLayout(headerLayout);
    layout->addWidget(switchAccountsLabel);
    layout->addWidget(yourChannelLabel);
    layout->addWidget(signOutLabel);
    layout->addStretch();
    layout->setSizeConstraint(QLayout::SetFixedSize);

    connect(switchAccountsLabel, &IconLabel::clicked, this, &AccountMenuWidget::accountSwitcherRequested);
    connect(signOutLabel, &IconLabel::clicked, this, &AccountMenuWidget::triggerSignOut);
    connect(yourChannelLabel, &IconLabel::clicked, this,
            std::bind(&AccountMenuWidget::gotoChannel, this, user->id, plugin));
}

void AccountMenuWidget::gotoChannel(const QString& channelId, PluginData* plugin)
{
    hide();
    ViewController::loadChannel(channelId, plugin);
    emit closeRequested();
}

void AccountMenuWidget::triggerSignOut()
{
    hide();
    MainWindow::topbar()->signOut();
    emit closeRequested();
}
