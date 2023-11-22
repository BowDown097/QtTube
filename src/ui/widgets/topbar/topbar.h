#ifndef TOPBAR_H
#define TOPBAR_H
#include "searchbox.h"
#include "topbarbell.h"
#include "ui/widgets/labels/tubelabel.h"
#include <QPushButton>

class QPropertyAnimation;

class TopBar : public QWidget
{
    Q_OBJECT
public:
    bool alwaysShow = true;
    QPropertyAnimation* animation;
    TubeLabel* avatarButton;
    TubeLabel* logo;
    TopBarBell* notificationBell;
    SearchBox* searchBox;
    TubeLabel* settingsButton;
    QPushButton* signInButton;

    explicit TopBar(QWidget* parent);
    void postSignInSetup();
    void scaleAppropriately();
    void updatePalette(const QPalette& palette);
public slots:
    void setUpAvatarButton();
    void setUpNotifications();
    void showSettings();
    void signOut();
    void trySignIn();
    void updateNotificationCount();
signals:
    void signInStatusChanged();
};

#endif // TOPBAR_H
