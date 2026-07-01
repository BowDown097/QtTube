#pragma once
#include "accountmenuwidget.hpp"
#include "accountswitcherwidget.hpp"
#include <QStackedWidget>

class AccountControllerWidget : public QStackedWidget
{
    Q_OBJECT
public:
    AccountMenuWidget* accountMenu;
    AccountSwitcherWidget* accountSwitcher;

    explicit AccountControllerWidget(PluginEntry* plugin, QWidget* parent = nullptr);
signals:
    void resized();
};
