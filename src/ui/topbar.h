#ifndef TOPBAR_H
#define TOPBAR_H
#include "clickablelabel.h"
#include <QLineEdit>
#include <QPropertyAnimation>
#include <QPushButton>

class TopBar : public QWidget
{
    Q_OBJECT
public:
    bool alwaysShow = true;
    QPropertyAnimation* animation;
    ClickableLabel* logo;
    ClickableLabel* notificationBell;
    QLabel* notificationCount;
    QLineEdit* searchBox;
    ClickableLabel* settingsButton;
    QPushButton* signInButton;
    explicit TopBar(QWidget* parent = nullptr);
    void scaleAppropriately();
public slots:
    void setUpNotifications();
    void showSettings();
    void signOut();
    void trySignIn();
    void updateNotificationCount();
private:
    bool preferDark;
signals:
    void notificationBellClicked();
    void signInStatusChanged();
};

#endif // TOPBAR_H
