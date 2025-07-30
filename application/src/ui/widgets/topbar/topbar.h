#pragma once
#include "searchbox.h"
#include "topbarbell.h"
#include "ui/widgets/labels/tubelabel.h"

class QPropertyAnimation;
class QPushButton;

class TopBar : public QWidget
{
    Q_OBJECT
public:
    TubeLabel* avatarButton;
    TubeLabel* logo;
    TopBarBell* notificationBell;
    SearchBox* searchBox;

    explicit TopBar(QWidget* parent);
    void handleMouseEvent(QMouseEvent* event);
    void postSignInSetup();
    void scaleAppropriately();
    void setAlwaysShow(bool alwaysShow) { this->alwaysShow = alwaysShow; }
    void updatePalette(const QPalette& palette);
private:
    bool alwaysShow = true;
    QPropertyAnimation* animation;
    TubeLabel* settingsButton;
    QPushButton* signInButton;
public slots:
    void signOut();
    void updateNotificationCount(int value);
private slots:
    void showSettings();
    void trySignIn();
signals:
    void signInStatusChanged();
};
