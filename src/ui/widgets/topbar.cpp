#include "topbar.h"
#include "innertube.h"
#include "settingsstore.h"
#include "ui/forms/settingsform.h"
#include "ui/uiutilities.h"
#include <QApplication>

TopBar::TopBar(QWidget* parent) : QWidget(parent), animation(new QPropertyAnimation(this, "geometry"))
{
    animation->setDuration(250);
    animation->setEasingCurve(QEasingCurve::InOutQuint);

    resize(parent->width(), 35);
    setAutoFillBackground(true);
    setPalette(qApp->palette().alternateBase().color());

    logo = new TubeLabel(this);
    logo->move(10, 2);
    logo->resize(134, 30);
    logo->setClickable(true, false);
    logo->setPixmap(QPixmap(UIUtilities::preferDark() ? ":/qttube-full-light.svg" : ":/qttube-full.svg"));
    logo->setScaledContents(true);

    notificationBell = new TubeLabel(this);
    notificationBell->resize(30, 30);
    notificationBell->setClickable(true, false);
    notificationBell->setScaledContents(true);
    connect(notificationBell, &TubeLabel::clicked, this, &TopBar::notificationBellClicked);

    notificationCount = new QLabel(this);
    notificationCount->setFont(QFont(qApp->font().toString(), 9));

    searchBox = new QLineEdit(this);
    searchBox->move(152, 0);
    searchBox->resize(513, 35);
    searchBox->setPlaceholderText("Search");

    settingsButton = new TubeLabel(this);
    settingsButton->move(673, 3);
    settingsButton->resize(30, 30);
    settingsButton->setClickable(true, false);
    settingsButton->setPixmap(QPixmap(UIUtilities::preferDark() ? ":/settings-light.svg" : ":/settings.svg"));
    settingsButton->setScaledContents(true);
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
    QSettings(SettingsStore::configPath.filePath("store.ini"), QSettings::IniFormat).clear();
}

void TopBar::trySignIn()
{
    if (InnerTube::instance().hasAuthenticated())
        return;

    InnerTube::instance().authenticate();
    if (InnerTube::instance().hasAuthenticated())
    {
        QSettings store(SettingsStore::configPath.filePath("store.ini"), QSettings::IniFormat);
        InnerTube::instance().authStore()->writeToSettings(store);

        setUpNotifications();
        signInButton->setText("Sign out");
        disconnect(signInButton, &QPushButton::clicked, this, &TopBar::trySignIn);
        connect(signInButton, &QPushButton::clicked, this, &TopBar::signOut);
        emit signInStatusChanged();
    }
}

void TopBar::updateNotificationCount()
{
    InnertubeReply* reply = InnerTube::instance().get<InnertubeEndpoints::UnseenCount>();
    connect(reply, qOverload<InnertubeEndpoints::UnseenCount>(&InnertubeReply::finished), this, [this](const auto& endpoint)
    {
        notificationBell->setPixmap(endpoint.unseenCount > 0
                                    ? QPixmap(UIUtilities::preferDark() ? ":/notif-bell-hasnotif-light.svg" : ":/notif-bell-hasnotif.svg")
                                    : QPixmap(UIUtilities::preferDark() ? ":/notif-bell-light.svg" : ":/notif-bell.svg"));
        notificationBell->setVisible(true);
        notificationCount->setText(QString::number(endpoint.unseenCount));
        notificationCount->setVisible(endpoint.unseenCount > 0);
    });
}

void TopBar::updatePalette(const QPalette& palette)
{
    setPalette(palette);
    logo->setPixmap(QPixmap(UIUtilities::preferDark() ? ":/qttube-full-light.svg" : ":/qttube-full.svg"));
    settingsButton->setPixmap(QPixmap(UIUtilities::preferDark() ? ":/settings-light.svg" : ":/settings.svg"));
    notificationBell->setPixmap(notificationCount->isVisible()
                                ? QPixmap(UIUtilities::preferDark() ? ":/notif-bell-hasnotif-light.svg" : ":/notif-bell-hasnotif.svg")
                                : QPixmap(UIUtilities::preferDark() ? ":/notif-bell-light.svg" : ":/notif-bell.svg"));
}
