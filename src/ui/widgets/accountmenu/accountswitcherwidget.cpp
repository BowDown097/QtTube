#include "accountswitcherwidget.h"
#include "accountentrywidget.h"
#include "innertube.h"
#include "ui/forms/mainwindow.h"

AccountSwitcherWidget::AccountSwitcherWidget(QWidget* parent)
    : QWidget(parent), addAccountButton(new QPushButton(this)), backButton(new QPushButton(this)), layout(new QVBoxLayout(this))
{
    addAccountButton->setText("Add account");
    backButton->setText("Back");
    layout->setSizeConstraint(QLayout::SetFixedSize);
    setAutoFillBackground(true);

    layout->addWidget(backButton);

    const CredentialSet& active = CredentialsStore::instance()->credentials[CredentialsStore::instance()->getActiveLoginIndex()];
    for (const CredentialSet& credSet : CredentialsStore::instance()->credentials)
    {
        AccountEntryWidget* accountEntry = new AccountEntryWidget(credSet, this);
        accountEntry->setClickable(credSet.channelId != active.channelId);
        connect(accountEntry, &AccountEntryWidget::clicked, this, std::bind(&AccountSwitcherWidget::switchAccount, this, credSet));
        layout->addWidget(accountEntry);
    }

    layout->addWidget(addAccountButton);

    connect(addAccountButton, &QPushButton::clicked, this, &AccountSwitcherWidget::addAccount);
    connect(backButton, &QPushButton::clicked, this, &AccountSwitcherWidget::accountMenuRequested);
}

void AccountSwitcherWidget::addAccount()
{
    setVisible(false);
    InnerTube::instance().unauthenticate();
    MainWindow::topbar()->trySignIn();
    emit closeRequested();
}

void AccountSwitcherWidget::switchAccount(const CredentialSet& credSet)
{
    setVisible(false);
    CredentialsStore::instance()->populateAuthStore(CredentialsStore::instance()->credentials.indexOf(credSet));
    MainWindow::topbar()->postSignInSetup();
    emit closeRequested();
}
