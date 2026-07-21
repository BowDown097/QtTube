#include "accountswitcherwidget.hpp"
#include "accountentrywidget.hpp"
#include "mainwindow.hpp"
#include "ui/widgets/topbar/topbar.hpp"
#include "utils/uiutils.hpp"
#include <QBoxLayout>
#include <QPushButton>
#include <qttube-plugin/providers/providertypes.h>

AccountSwitcherWidget::AccountSwitcherWidget(PluginEntry* plugin, QWidget* parent)
    : QWidget(parent),
      m_addAccountButton(new QPushButton(this)),
      m_backButton(new QPushButton(this)),
      m_layout(new QVBoxLayout(this)),
      m_plugin(plugin)
{
    assert(plugin->authStore != nullptr);

    m_layout->setSizeConstraint(QLayout::SetFixedSize);
    setAutoFillBackground(true);

    m_backButton->setText("Back");
    m_layout->addWidget(m_backButton);

    QtTubePlugin::AuthUser* activeUser = plugin->authStore->activeBaseLogin();
    if (!activeUser)
        throw std::runtime_error("Account switcher somehow opened without an active login.");

    for (const std::unique_ptr<QtTubePlugin::AuthUser>& user : plugin->authStore->baseCredentials())
    {
        AccountEntryWidget* accountEntry = new AccountEntryWidget(*user, this);
        accountEntry->setClickable(user->id != activeUser->id);
        connect(accountEntry, &AccountEntryWidget::clicked, this,
            std::bind(&AccountSwitcherWidget::switchAccount, this, activeUser, user.get()));
        m_layout->addWidget(accountEntry);
    }

    m_addAccountButton->setText("Add account");
    m_layout->addWidget(m_addAccountButton);

    connect(m_addAccountButton, &QPushButton::clicked, this, &AccountSwitcherWidget::addAccount);
    connect(m_backButton, &QPushButton::clicked, this, &AccountSwitcherWidget::accountMenuRequested);
}

void AccountSwitcherWidget::addAccount()
{
    hide();
    m_plugin->authStore->unauthenticate();
    m_plugin->authStore->startAuthRoutine();
    emit closeRequested();
}

void AccountSwitcherWidget::switchAccount(QtTubePlugin::AuthUser* oldUser, QtTubePlugin::AuthUser* newUser)
{
    oldUser->active = false;
    newUser->active = true;

    hide();
    m_plugin->authStore->restoreFromActive();
    UIUtils::getMainWindow()->topbar()->postSignInSetup(m_plugin);
    emit closeRequested();
}
