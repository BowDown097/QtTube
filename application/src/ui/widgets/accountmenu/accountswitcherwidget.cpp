#include "accountswitcherwidget.h"
#include "accountentrywidget.h"
#include "mainwindow.h"
#include "plugins/pluginmanager.h"
#include "ui/widgets/topbar/topbar.h"
#include "utils/uiutils.h"
#include <QBoxLayout>
#include <QPushButton>

AccountSwitcherWidget::AccountSwitcherWidget(PluginData* plugin, QWidget* parent)
    : QWidget(parent),
      m_addAccountButton(new QPushButton(this)),
      m_backButton(new QPushButton(this)),
      m_layout(new QVBoxLayout(this))
{
    m_addAccountButton->setText("Add account");
    m_backButton->setText("Back");
    m_layout->setSizeConstraint(QLayout::SetFixedSize);
    setAutoFillBackground(true);

    m_layout->addWidget(m_backButton);

    if (!(m_auth = plugin->auth))
        throw std::runtime_error("Account switcher somehow opened without auth support.");

    QtTubePlugin::AuthUser* activeUser = plugin->auth->activeBaseLogin();
    if (!activeUser)
        throw std::runtime_error("Account switcher somehow opened without an active login.");

    for (QtTubePlugin::AuthUser* user : plugin->auth->baseCredentials())
    {
        AccountEntryWidget* accountEntry = new AccountEntryWidget(*user, this);
        accountEntry->setClickable(user->id != activeUser->id);
        connect(accountEntry, &AccountEntryWidget::clicked, this, [this, activeUser, user] { switchAccount(activeUser, user); });
        m_layout->addWidget(accountEntry);
    }

    m_layout->addWidget(m_addAccountButton);

    connect(m_addAccountButton, &QPushButton::clicked, this, &AccountSwitcherWidget::addAccount);
    connect(m_backButton, &QPushButton::clicked, this, &AccountSwitcherWidget::accountMenuRequested);
}

void AccountSwitcherWidget::addAccount()
{
    hide();
    m_auth->unauthenticate();
    m_auth->startAuthRoutine();
    emit closeRequested();
}

void AccountSwitcherWidget::switchAccount(QtTubePlugin::AuthUser* oldUser, QtTubePlugin::AuthUser* newUser)
{
    oldUser->active = false;
    newUser->active = true;

    hide();
    m_auth->restoreFromActive();
    UIUtils::getMainWindow()->topbar()->postSignInSetup();
    emit closeRequested();
}
