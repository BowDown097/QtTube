#include "accountcontrollerwidget.h"

AccountControllerWidget::AccountControllerWidget(PluginData* plugin, QWidget* parent)
    : QStackedWidget(parent),
      accountMenu(new AccountMenuWidget(plugin)),
      accountSwitcher(new AccountSwitcherWidget(plugin))
{
    addWidget(accountMenu);
    addWidget(accountSwitcher);
    adjustSize();

    connect(accountMenu, &AccountMenuWidget::accountSwitcherRequested, this, [this] { setCurrentIndex(1); });
    connect(accountMenu, &AccountMenuWidget::closeRequested, this, &AccountControllerWidget::deleteLater);
    connect(accountSwitcher, &AccountSwitcherWidget::accountMenuRequested, this, [this] { setCurrentIndex(0); });
    connect(accountSwitcher, &AccountSwitcherWidget::closeRequested, this, &AccountControllerWidget::deleteLater);
}
