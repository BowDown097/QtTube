#include "accountswitcherwidget.h"
#include "accountentrywidget.h"
#include "innertube.h"
#include "mainwindow.h"
#include "qttubeapplication.h"
#include <QBoxLayout>
#include <QPushButton>

AccountSwitcherWidget::AccountSwitcherWidget(QWidget* parent)
    : QWidget(parent), addAccountButton(new QPushButton(this)), backButton(new QPushButton(this)), layout(new QVBoxLayout(this))
{
    addAccountButton->setText("Add account");
    backButton->setText("Back");
    layout->setSizeConstraint(QLayout::SetFixedSize);
    setAutoFillBackground(true);

    layout->addWidget(backButton);

    CredentialSet activeLogin = qtTubeApp->creds().activeLogin();
    for (const CredentialSet& credSet : qtTubeApp->creds().credentials())
    {
        AccountEntryWidget* accountEntry = new AccountEntryWidget(credSet, this);
        accountEntry->setClickable(credSet.channelId != activeLogin.channelId);
        connect(accountEntry, &AccountEntryWidget::clicked, this, std::bind(&AccountSwitcherWidget::switchAccount, this, credSet));
        layout->addWidget(accountEntry);
    }

    layout->addWidget(addAccountButton);

    connect(addAccountButton, &QPushButton::clicked, this, &AccountSwitcherWidget::addAccount);
    connect(backButton, &QPushButton::clicked, this, &AccountSwitcherWidget::accountMenuRequested);
}

void AccountSwitcherWidget::addAccount()
{
    hide();
    InnerTube::instance().unauthenticate();
    MainWindow::topbar()->trySignIn();
    emit closeRequested();
}

void AccountSwitcherWidget::switchAccount(const CredentialSet& credSet)
{
    hide();
    qtTubeApp->creds().populateAuthStore(credSet);
    MainWindow::topbar()->postSignInSetup();
    emit closeRequested();
}
