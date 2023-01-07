#include "topbar.h"
#include "innertube.h"
#include "settingsstore.h"
#include "ui/forms/mainwindow.h"
#include "ui/forms/settingsform.h"
#include <QApplication>
#include <QJsonDocument>

TopBar::TopBar(QWidget* parent) : QWidget(parent), animation(new QPropertyAnimation(this, "geometry"))
{
    animation->setDuration(250);
    animation->setEasingCurve(QEasingCurve::InOutQuint);
    resize(MainWindow::instance()->width(), 35);

    const QColor& aBase = QApplication::palette().color(QPalette::AlternateBase);
    preferDark = aBase.lightness() < 60;

    QPalette pal;
    pal.setColor(QPalette::Window, aBase);
    setAutoFillBackground(true);
    setPalette(pal);

    logo = new TubeLabel(this);
    logo->move(10, 2);
    logo->resize(134, 30);
    logo->setClickable(true, false);
    logo->setPixmap(QPixmap(preferDark ? ":/qttube-full-light.png" : ":/qttube-full.png"));

    notificationBell = new TubeLabel(this);
    notificationBell->resize(30, 30);
    notificationBell->setClickable(true, false);
    connect(notificationBell, &TubeLabel::clicked, this, [this] { emit notificationBellClicked(); });

    notificationCount = new QLabel(this);
    notificationCount->setFont(QFont(QApplication::font().toString(), 9));

    searchBox = new QLineEdit(this);
    searchBox->move(152, 0);
    searchBox->resize(513, 35);
    searchBox->setPlaceholderText("Search");

    settingsButton = new TubeLabel(this);
    settingsButton->move(673, 3);
    settingsButton->resize(30, 30);
    settingsButton->setClickable(true, false);
    settingsButton->setPixmap(QPixmap(preferDark ? ":/settings-light.png" : ":/settings.png"));
    connect(settingsButton, &TubeLabel::clicked, this, &TopBar::showSettings);

    signInButton = new QPushButton(this);
    signInButton->move(711, 0);
    signInButton->resize(80, 35);
    signInButton->setText("Sign in");
    connect(signInButton, &QPushButton::clicked, this, &TopBar::trySignIn);
}

void TopBar::scaleAppropriately()
{
    if (InnerTube::instance().hasAuthenticated())
    {
        searchBox->resize(452 + width() - 800, 35);
        notificationBell->move(searchBox->width() + searchBox->x() + 8, 2);
        notificationCount->move(notificationBell->x() + 20, 0);
        settingsButton->move(notificationBell->width() + notificationBell->x() + 8, 4);
        signInButton->move(settingsButton->width() + settingsButton->x() + 8, 0);
    }
    else
    {
        searchBox->resize(490 + width() - 800, 35);
        settingsButton->move(searchBox->width() + searchBox->x() + 8, 4);
        signInButton->move(settingsButton->width() + settingsButton->x() + 8, 0);
    }
}

void TopBar::setUpNotifications()
{
    scaleAppropriately();
    if (InnerTube::instance().hasAuthenticated())
    {
        updateNotificationCount();
    }
    else
    {
        notificationBell->setVisible(false);
        notificationCount->setVisible(false);
    }
}

void TopBar::showSettings()
{
    SettingsForm* settings = new SettingsForm;
    settings->show();
}

void TopBar::signOut()
{
    InnerTube::instance().unauthenticate();
    setUpNotifications();
    signInButton->setText("Sign in");
    disconnect(signInButton, &QPushButton::clicked, this, &TopBar::signOut);
    connect(signInButton, &QPushButton::clicked, this, &TopBar::trySignIn);
    emit signInStatusChanged();

    if (SettingsStore::instance().itcCache)
        QFile(SettingsStore::configPath.filePath("store.json")).resize(0);
}

void TopBar::trySignIn()
{
    if (InnerTube::instance().hasAuthenticated())
        return;

    InnerTube::instance().authenticate();
    if (SettingsStore::instance().itcCache)
    {
        QFile store(SettingsStore::configPath.filePath("store.json"));
        if (store.open(QFile::WriteOnly | QFile::Text))
        {
            QTextStream storeIn(&store);
            storeIn << QJsonDocument(InnerTube::instance().authStore()->toJson()).toJson(QJsonDocument::Compact);
        }
    }

    if (InnerTube::instance().hasAuthenticated())
    {
        setUpNotifications();
        signInButton->setText("Sign out");
        disconnect(signInButton, &QPushButton::clicked, this, &TopBar::trySignIn);
        connect(signInButton, &QPushButton::clicked, this, &TopBar::signOut);
        emit signInStatusChanged();
    }
}

void TopBar::updateColors(const QColor& color)
{
    QPalette pal;
    pal.setColor(QPalette::Window, color);
    setPalette(pal);

    preferDark = color.lightness() < 60;
    logo->setPixmap(QPixmap(preferDark ? ":/qttube-full-light.png" : ":/qttube-full.png"));
    settingsButton->setPixmap(QPixmap(preferDark ? ":/settings-light.png" : ":/settings.png"));
    updateNotificationCount();
}

void TopBar::updateNotificationCount()
{
    int unseenCount = InnerTube::instance().get<InnertubeEndpoints::UnseenCount>().unseenCount;
    notificationBell->setPixmap(unseenCount > 0
                                ? QPixmap(preferDark ? ":/notif-bell-hasnotif-light.png" : ":/notif-bell-hasnotif.png")
                                : QPixmap(preferDark ? ":/notif-bell-light.png" : ":/notif-bell.png"));
    notificationBell->setVisible(true);
    notificationCount->setText(QString::number(unseenCount));
    notificationCount->setVisible(unseenCount > 0);
}