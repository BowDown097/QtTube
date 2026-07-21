#pragma once
#include "plugins/pluginentry.hpp"
#include <QWidget>

class QPushButton;
class QVBoxLayout;

class AccountSwitcherWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AccountSwitcherWidget(PluginEntry* plugin, QWidget* parent = nullptr);
private:
    QPushButton* m_addAccountButton;
    QPushButton* m_backButton;
    QVBoxLayout* m_layout;
    PluginEntry* m_plugin;
private slots:
    void addAccount();
    void switchAccount(QtTubePlugin::AuthUser* oldUser, QtTubePlugin::AuthUser* newUser);
signals:
    void accountMenuRequested();
    void closeRequested();
};
