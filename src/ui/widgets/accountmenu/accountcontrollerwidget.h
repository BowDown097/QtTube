#ifndef ACCOUNTCONTROLLERWIDGET_H
#define ACCOUNTCONTROLLERWIDGET_H
#include "accountmenuwidget.h"
#include "accountswitcherwidget.h"
#include <QStackedWidget>

class AccountControllerWidget : public QStackedWidget
{
    Q_OBJECT
public:
    AccountMenuWidget* accountMenu;
    AccountSwitcherWidget* accountSwitcher;
    explicit AccountControllerWidget(QWidget* parent = nullptr);
signals:
    void resized();
};

#endif // ACCOUNTCONTROLLERWIDGET_H
