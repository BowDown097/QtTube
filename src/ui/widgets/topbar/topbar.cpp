// TODO: make this whole mess better. i'm really bad at qt gui and
// can't figure out how to make this work with an hbox layout, which
// is what this should be using instead of whatever the hell's going on.

#include "topbar.h"
#include "http.h"
#include "innertube.h"
#include "stores/credentialsstore.h"
#include "ui/forms/settings/settingsform.h"
#include "utils/uiutils.h"
#include <QApplication>

#ifdef INNERTUBE_NO_WEBENGINE
#include <QMessageBox>
#endif

TopBar::TopBar(QWidget* parent)
    : QWidget(parent),
      animation(new QPropertyAnimation(this, "geometry")),
      avatarButton(new TubeLabel(this)),
      logo(new TubeLabel(this)),
      notificationBell(new TopBarBell(this)),
      searchBox(new SearchBox(this)),
      settingsButton(new TubeLabel(this)),
      signInButton(new QPushButton(this))
{
    resize(parent->width(), 35);
    setAutoFillBackground(true);
    setPalette(qApp->palette().alternateBase().color());

    animation->setDuration(250);
    animation->setEasingCurve(QEasingCurve::InOutQuint);

    avatarButton->move(673, 3);
    avatarButton->resize(30, 30);
    avatarButton->setClickable(true, false);
    avatarButton->setVisible(false);

    logo->move(10, 2);
    logo->resize(134, 30);
    logo->setClickable(true, false);
    logo->setPixmap(UIUtils::pixmapThemed("qttube-full", true, logo->size()));
    logo->setScaledContents(true);

    searchBox->move(152, 0);
    searchBox->resize(513, 35);

    settingsButton->move(673, 3);
    settingsButton->resize(30, 30);
    settingsButton->setClickable(true, false);
    settingsButton->setPixmap(UIUtils::pixmapThemed("settings"));
    settingsButton->setScaledContents(true);
    connect(settingsButton, &TubeLabel::clicked, this, &TopBar::showSettings);

    signInButton->move(711, 0);
    signInButton->resize(80, 35);
    signInButton->setText("Sign in");
    connect(signInButton, &QPushButton::clicked, this, &TopBar::trySignIn);
}

void TopBar::postSignInSetup()
{
    avatarButton->setVisible(true);
    signInButton->setVisible(false);
    setUpAvatarButton();
    setUpNotifications();
    emit signInStatusChanged();
}

void TopBar::scaleAppropriately()
{
    if (InnerTube::instance().hasAuthenticated())
    {
        searchBox->resize(502 + width() - 800, 35);
        notificationBell->move(searchBox->width() + searchBox->x() + 8, 2);
        settingsButton->move(notificationBell->width() + notificationBell->x() + 8, 4);
        avatarButton->move(settingsButton->width() + settingsButton->x() + 8, 3);
    }
    else
    {
        searchBox->resize(490 + width() - 800, 35);
        settingsButton->move(searchBox->width() + searchBox->x() + 8, 4);
        signInButton->move(settingsButton->width() + settingsButton->x() + 8, 0);
    }
}

void TopBar::setUpAvatarButton()
{
    scaleAppropriately();
    InnertubeReply* reply = InnerTube::instance().get<InnertubeEndpoints::AccountMenu>();
    connect(reply, qOverload<const InnertubeEndpoints::AccountMenu&>(&InnertubeReply::finished), this, [this](const InnertubeEndpoints::AccountMenu& endpoint)
    {
        CredentialsStore::instance()->updateAccount(endpoint);
        HttpReply* photoReply = Http::instance().get(QUrl(endpoint.response.header.accountPhotos[0].url));
        connect(photoReply, &HttpReply::finished, this, [this](const HttpReply& reply)
        {
            QPixmap pixmap;
            pixmap.loadFromData(reply.body());
            pixmap = pixmap.scaled(avatarButton->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            avatarButton->setPixmap(UIUtils::pixmapRounded(pixmap, 15, 15));
        });
    });
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
    avatarButton->setVisible(false);
    signInButton->setVisible(true);
    emit signInStatusChanged();
    QSettings(CredentialsStore::configPath, QSettings::IniFormat).clear();
}

void TopBar::trySignIn()
{
#ifndef INNERTUBE_NO_WEBENGINE
    InnerTube::instance().authenticate();
    if (!InnerTube::instance().hasAuthenticated())
        return;
#else
    QMessageBox::StandardButton box = QMessageBox::information(nullptr, "YouTube Login",
R"(
Could not bring up the YouTube login page because the Qt web engine is not available.
You will need to provide authentication credentials manually to log in.
For info on how to do this, see https://github.com/BowDown097/innertube-qt/wiki/Manually-getting-login-credentials.
)");
    if (box != QMessageBox::StandardButton::Ok)
        return;

    int index = CredentialsStore::instance()->getActiveLoginIndex();
    if (index == -1)
        index = 0;

    CredentialsStore::instance()->populateAuthStore(index);
    if (!InnerTube::instance().hasAuthenticated())
    {
        QMessageBox::information(nullptr, "Not Logged In",
R"(
You didn't provide authentication credentials or the credentials you provided are invalid.
If you provided credentials, please check them, refer back to the previous linked guide if needed, and try again.
)");
        return;
    }
#endif

    postSignInSetup();
}

void TopBar::updateNotificationCount()
{
    InnertubeReply* reply = InnerTube::instance().get<InnertubeEndpoints::UnseenCount>();
    connect(reply, qOverload<const InnertubeEndpoints::UnseenCount&>(&InnertubeReply::finished), this, [this](const InnertubeEndpoints::UnseenCount& endpoint)
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
    logo->setPixmap(UIUtils::pixmapThemed("qttube-full", true, logo->size(), palette));
    notificationBell->updatePixmap(notificationBell->count->isVisible(), palette);
    searchBox->updatePalette(palette);
    settingsButton->setPixmap(UIUtils::pixmapThemed("settings", false, QSize(), palette));

    if (!InnerTube::instance().hasAuthenticated())
        notificationBell->setVisible(false);
}
