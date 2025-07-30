#pragma once
#include "qttube-plugin/components/auth/authstore.h"
#include <QWidget>

class QPushButton;
class QVBoxLayout;

class AccountSwitcherWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AccountSwitcherWidget(QWidget* parent = nullptr);
private:
    QPushButton* addAccountButton;
    QtTubePlugin::AuthStoreBase* auth;
    QPushButton* backButton;
    QVBoxLayout* layout;
private slots:
    void addAccount();
    void switchAccount(QtTubePlugin::AuthUser* oldUser, QtTubePlugin::AuthUser* newUser);
signals:
    void accountMenuRequested();
    void closeRequested();
};
