#pragma once
#include <QWidget>

namespace InnertubeEndpoints { struct AccountMenu; }

class IconLabel;
class QHBoxLayout;
class QVBoxLayout;
class TubeLabel;

class AccountMenuWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AccountMenuWidget(QWidget* parent = nullptr);
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
public slots:
    void initialize(const InnertubeEndpoints::AccountMenu& endpoint);
private slots:
    void gotoChannel(const QString& channelId);
    void triggerSignOut();
signals:
    void accountSwitcherRequested();
    void closeRequested();
    void finishedInitializing();
};
