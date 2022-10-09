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
    QLineEdit* searchBox;
    QPushButton* settingsButton;
    QPushButton* signInButton;
    explicit TopBar(QWidget* parent = nullptr);
    ~TopBar();
public slots:
    void showSettings();
    void trySignIn();
signals:
    void signedIn();
};

#endif // TOPBAR_H
