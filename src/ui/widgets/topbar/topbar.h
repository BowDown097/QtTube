#ifndef TOPBAR_H
#define TOPBAR_H
#include "topbarbell.h"
#include "ui/widgets/labels/tubelabel.h"
#include "ui/widgets/qiconwidget.h"
#include <QLineEdit>
#include <QPropertyAnimation>
#include <QPushButton>

class TopBar : public QWidget
{
    Q_OBJECT
public:
    bool alwaysShow = true;
    QPropertyAnimation* animation;
    TubeLabel* avatarButton;
    QIconWidget* logo;
    TopBarBell* notificationBell;
    QLineEdit* searchBox;
    QIconWidget* settingsButton;
    QPushButton* signInButton;
    explicit TopBar(QWidget* parent);
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
