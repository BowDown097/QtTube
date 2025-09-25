#pragma once
#include <QWidget>

class IconLabel;
struct PluginData;
class QHBoxLayout;
class QVBoxLayout;
class TubeLabel;

class AccountMenuWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AccountMenuWidget(PluginData* plugin, QWidget* parent = nullptr);
private:
    QVBoxLayout* accountLayout;
    TubeLabel* accountNameLabel;
    TubeLabel* avatar;
    TubeLabel* handleLabel;
    QHBoxLayout* headerLayout;
    QVBoxLayout* layout;
    IconLabel* signOutLabel;
    IconLabel* switchAccountsLabel;
    IconLabel* yourChannelLabel;
private slots:
    void gotoChannel(const QString& channelId, PluginData* plugin);
    void triggerSignOut();
signals:
    void accountSwitcherRequested();
    void closeRequested();
};
