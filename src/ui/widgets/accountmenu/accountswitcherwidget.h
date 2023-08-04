#ifndef ACCOUNTSWITCHERWIDGET_H
#define ACCOUNTSWITCHERWIDGET_H
#include "stores/credentialsstore.h"
#include <QPushButton>
#include <QVBoxLayout>

class AccountSwitcherWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AccountSwitcherWidget(QWidget* parent = nullptr);
private:
    QPushButton* addAccountButton;
    QPushButton* backButton;
    QVBoxLayout* layout;
private slots:
    void addAccount();
    void switchAccount(const CredentialSet& credSet);
signals:
    void accountMenuRequested();
    void closeRequested();
};

#endif // ACCOUNTSWITCHERWIDGET_H
