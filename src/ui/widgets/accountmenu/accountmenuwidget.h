#pragma once
#include <QWidget>

namespace InnertubeEndpoints { struct AccountMenu; }

class HttpReply;
class IconLabel;
class QHBoxLayout;
class QLabel;
class QVBoxLayout;

class AccountMenuWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AccountMenuWidget(QWidget* parent = nullptr);
private:
    QVBoxLayout* accountLayout;
    QLabel* accountNameLabel;
    QLabel* avatar;
    QLabel* handleLabel;
    QHBoxLayout* headerLayout;
    QVBoxLayout* layout;
    IconLabel* signOutLabel;
    IconLabel* switchAccountsLabel;
    IconLabel* yourChannelLabel;
public slots:
    void initialize(const InnertubeEndpoints::AccountMenu& endpoint);
private slots:
    void gotoChannel(const QString& channelId);
    void setAvatar(const HttpReply& reply);
    void triggerSignOut();
signals:
    void accountSwitcherRequested();
    void closeRequested();
    void finishedInitializing();
};
