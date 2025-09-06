#include "accountswitcherwidget.h"
#include "accountentrywidget.h"
#include "mainwindow.h"
#include "plugins/pluginmanager.h"
#include <QBoxLayout>
#include <QPushButton>

AccountSwitcherWidget::AccountSwitcherWidget(PluginData* plugin, QWidget* parent)
    : QWidget(parent),
      addAccountButton(new QPushButton(this)),
      backButton(new QPushButton(this)),
      layout(new QVBoxLayout(this))
{
    addAccountButton->setText("Add account");
    backButton->setText("Back");
    layout->setSizeConstraint(QLayout::SetFixedSize);
    setAutoFillBackground(true);

    layout->addWidget(backButton);

    if (!(this->auth = plugin->auth))
        throw std::runtime_error("Account switcher somehow opened without auth support.");

    QtTubePlugin::AuthUser* activeUser = plugin->auth->activeBaseLogin();
    if (!activeUser)
        throw std::runtime_error("Account switcher somehow opened without an active login.");

    for (QtTubePlugin::AuthUser* user : plugin->auth->baseCredentials())
    {
        AccountEntryWidget* accountEntry = new AccountEntryWidget(*user, this);
        accountEntry->setClickable(user->id != activeUser->id);
        connect(accountEntry, &AccountEntryWidget::clicked, this, [this, activeUser, user] { switchAccount(activeUser, user); });
        layout->addWidget(accountEntry);
    }

    layout->addWidget(addAccountButton);

    connect(addAccountButton, &QPushButton::clicked, this, &AccountSwitcherWidget::addAccount);
    connect(backButton, &QPushButton::clicked, this, &AccountSwitcherWidget::accountMenuRequested);
}

void AccountSwitcherWidget::addAccount()
{
    hide();
    auth->unauthenticate();
    auth->startAuthRoutine();
    emit closeRequested();
}

void AccountSwitcherWidget::switchAccount(QtTubePlugin::AuthUser* oldUser, QtTubePlugin::AuthUser* newUser)
{
    oldUser->active = false;
    newUser->active = true;

    hide();
    auth->restoreFromActive();
    MainWindow::topbar()->postSignInSetup();
    emit closeRequested();
}
