#include "accountcontrollerwidget.h"

AccountControllerWidget::AccountControllerWidget(QWidget* parent)
    : QStackedWidget(parent), accountMenu(new AccountMenuWidget), accountSwitcher(new AccountSwitcherWidget)
{
    addWidget(accountMenu);
    addWidget(accountSwitcher);

    connect(accountMenu, &AccountMenuWidget::accountSwitcherRequested, this, [this] { setCurrentIndex(1); });
    connect(accountMenu, &AccountMenuWidget::closeRequested, this, &AccountControllerWidget::deleteLater);
    connect(accountMenu, &AccountMenuWidget::finishedInitializing, this, [this] { adjustSize(); emit resized(); });
    connect(accountSwitcher, &AccountSwitcherWidget::accountMenuRequested, this, [this] { setCurrentIndex(0); });
    connect(accountSwitcher, &AccountSwitcherWidget::closeRequested, this, &AccountControllerWidget::deleteLater);
}
