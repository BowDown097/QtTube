#ifndef TOPBAR_H
#define TOPBAR_H
#include "tubelabel.h"
#include <QLineEdit>
#include <QPropertyAnimation>
#include <QPushButton>

class TopBar : public QWidget
{
    Q_OBJECT
public:
    bool alwaysShow = true;
    QPropertyAnimation* animation;
    TubeLabel* logo;
    TubeLabel* notificationBell;
    QLabel* notificationCount;
    QLineEdit* searchBox;
    TubeLabel* settingsButton;
    QPushButton* signInButton;
    explicit TopBar(QWidget* parent);
    void scaleAppropriately();
    void updatePalette(const QPalette& palette);
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
