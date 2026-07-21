#pragma once
#include "plugins/pluginentry.hpp"
#include <QWidget>

class IconLabel;
class QHBoxLayout;
class QVBoxLayout;
class TubeLabel;

class AccountMenuWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AccountMenuWidget(PluginEntry* plugin, QWidget* parent = nullptr);
private:
    QVBoxLayout* m_accountLayout;
    TubeLabel* m_accountNameLabel;
    TubeLabel* m_avatar;
    TubeLabel* m_handleLabel;
    QHBoxLayout* m_headerLayout;
    QVBoxLayout* m_layout;
    PluginEntry* m_plugin;
    IconLabel* m_signOutLabel;
    IconLabel* m_switchAccountsLabel;
    IconLabel* m_yourChannelLabel;
private slots:
    void gotoChannel(const QString& channelId);
    void triggerSignOut();
signals:
    void accountSwitcherRequested();
    void closeRequested();
};
