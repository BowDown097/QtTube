#include "topbar.h"
#include "innertube.h"
#include "settingsstore.h"
#include "ui/forms/settingsform.h"
#include "ui/uiutilities.h"
#include <QApplication>

#ifdef INNERTUBE_NO_WEBENGINE
#include <QMessageBox>
#endif

TopBar::TopBar(QWidget* parent)
    : QWidget(parent),
      animation(new QPropertyAnimation(this, "geometry")),
      logo(new TubeLabel(this)),
      notificationBell(new TopBarBell(this)),
      searchBox(new QLineEdit(this)),
      settingsButton(new TubeLabel(this)),
      signInButton(new QPushButton(this))
{
    animation->setDuration(250);
    animation->setEasingCurve(QEasingCurve::InOutQuint);

    resize(parent->width(), 35);
    setAutoFillBackground(true);
    setPalette(qApp->palette().alternateBase().color());

    logo->move(10, 2);
    logo->resize(134, 30);
    logo->setClickable(true, false);
    logo->setPixmap(UIUtilities::icon("qttube-full", true, logo->size()));
    logo->setScaledContents(true);

    searchBox->move(152, 0);
    searchBox->resize(513, 35);
    searchBox->setPlaceholderText("Search");

    settingsButton->move(673, 3);
    settingsButton->resize(30, 30);
    settingsButton->setClickable(true, false);
    settingsButton->setPixmap(UIUtilities::icon("settings"));
    settingsButton->setScaledContents(true);
    connect(settingsButton, &TubeLabel::clicked, this, &TopBar::showSettings);

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
        updateNotificationCount();
    else
        notificationBell->setVisible(false);
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

#ifndef INNERTUBE_NO_WEBENGINE
    InnerTube::instance().authenticate();
    if (!InnerTube::instance().hasAuthenticated())
        return;

    QSettings store(SettingsStore::configPath.filePath("store.ini"), QSettings::IniFormat);
    InnerTube::instance().authStore()->writeToSettings(store);
#else
    QMessageBox::StandardButton box = QMessageBox::information(nullptr, "YouTube Login",
R"(
Could not bring up the YouTube login page because the Qt web engine is not available.
You will need to provide authentication credentials manually to log in.
For info on how to do this, see https://github.com/BowDown097/innertube-qt/wiki/Manually-getting-login-credentials.
)");
    if (box != QMessageBox::StandardButton::Ok)
        return;

    QSettings store(SettingsStore::configPath.filePath("store.ini"), QSettings::IniFormat);
    InnerTube::instance().authenticateFromSettings(store);
    if (!InnerTube::instance().hasAuthenticated())
    {
        QMessageBox::information(nullptr, "Not Logged In",
R"(
You didn't provide authentication credentials or the credentials you provided are invalid.
If you provided credentials, please check them, refer back to the previous linked guide if needed, and try again.
)");
        return;
    }

    SettingsStore::instance().saveToSettingsFile();
#endif

    setUpNotifications();
    signInButton->setText("Sign out");
    disconnect(signInButton, &QPushButton::clicked, this, &TopBar::trySignIn);
    connect(signInButton, &QPushButton::clicked, this, &TopBar::signOut);
    emit signInStatusChanged();
}

void TopBar::updateNotificationCount()
{
    InnertubeReply* reply = InnerTube::instance().get<InnertubeEndpoints::UnseenCount>();
    connect(reply, qOverload<InnertubeEndpoints::UnseenCount>(&InnertubeReply::finished), this, [this](const auto& endpoint)
    {
        notificationBell->updatePixmap(endpoint.unseenCount > 0, palette());
        notificationBell->setVisible(true);
        notificationBell->count->setText(QString::number(endpoint.unseenCount));
        notificationBell->count->setVisible(endpoint.unseenCount > 0);
    });
}

void TopBar::updatePalette(const QPalette& palette)
{
    setPalette(palette);
    logo->setPixmap(UIUtilities::icon("qttube-full", true, logo->size(), palette));
    settingsButton->setPixmap(UIUtilities::icon("settings", false, QSize(), palette));
    notificationBell->updatePixmap(notificationBell->count->isVisible(), palette);
}
