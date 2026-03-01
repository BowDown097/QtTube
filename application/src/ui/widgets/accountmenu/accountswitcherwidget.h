#pragma once
#include "qttube-plugin/components/auth/authstore.h"
#include <QWidget>

class PluginEntry;
class QPushButton;
class QVBoxLayout;

class AccountSwitcherWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AccountSwitcherWidget(PluginEntry* plugin, QWidget* parent = nullptr);
private:
    QPushButton* m_addAccountButton;
    QtTubePlugin::AuthStoreBase* m_auth;
    QPushButton* m_backButton;
    QVBoxLayout* m_layout;
private slots:
    void addAccount();
    void switchAccount(QtTubePlugin::AuthUser* oldUser, QtTubePlugin::AuthUser* newUser);
signals:
    void accountMenuRequested();
    void closeRequested();
};
