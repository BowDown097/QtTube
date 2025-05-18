// TODO: make this whole mess better. i'm really bad at qt gui and
// can't figure out how to make this work with an hbox layout, which
// is what this should be using instead of whatever the hell's going on.

#include "topbar.h"
#include "innertube.h"
#include "qttubeapplication.h"
#include "ui/forms/settings/settingsform.h"
#include "utils/uiutils.h"
#include <QApplication>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QTabBar>

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

    avatarButton->hide();
    avatarButton->move(673, 3);
    avatarButton->resize(30, 30);
    avatarButton->setClickable(true);
    avatarButton->setScaledContents(true);

    logo->move(10, 2);
    logo->resize(134, 30);
    logo->setClickable(true);
    logo->setPixmap(UIUtils::iconThemed("qttube-full").pixmap(logo->size()));
    logo->setScaledContents(true);

    searchBox->move(152, 0);
    searchBox->resize(513, 35);

    settingsButton->move(673, 3);
    settingsButton->resize(30, 30);
    settingsButton->setClickable(true);
    settingsButton->setPixmap(UIUtils::pixmapThemed("settings"));
    settingsButton->setScaledContents(true);
    connect(settingsButton, &TubeLabel::clicked, this, &TopBar::showSettings);

    signInButton->move(711, 0);
    signInButton->resize(80, 35);
    signInButton->setText("Sign in");
    connect(signInButton, &QPushButton::clicked, this, &TopBar::trySignIn);
}

void TopBar::handleMouseEvent(QMouseEvent* event)
{
    bool interferingWithTab{};
    if (QWidget* widgetAtPoint = qApp->widgetAt(QCursor::pos()))
        interferingWithTab = strncmp(widgetAtPoint->metaObject()->className(), "QTab", 4) == 0;
    if (alwaysShow || animation->state() == QAbstractAnimation::Running || interferingWithTab)
        return;

    if (event->pos().y() < height())
    {
        if (isHidden())
        {
            animation->setStartValue(QRect(0, 0, width(), 0));
            animation->setEndValue(QRect(0, 0, width(), height()));
            disconnect(animation, &QPropertyAnimation::finished, this, nullptr);
            animation->start();
            show();
        }
    }
    else if (isVisible())
    {
        animation->setStartValue(QRect(0, 0, width(), height()));
        animation->setEndValue(QRect(0, 0, width(), 0));
        connect(animation, &QPropertyAnimation::finished, this, [this] {
            hide();
            resize(width(), animation->startValue().toRect().height());
        });
        animation->start();
    }
}

void TopBar::postSignInSetup(bool emitSignal)
{
    avatarButton->show();
    signInButton->hide();
    setUpAvatarButton();
    setUpNotifications();

    if (emitSignal)
        emit signInStatusChanged();
}

void TopBar::scaleAppropriately()
{
    if (InnerTube::instance()->hasAuthenticated())
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
    auto reply = InnerTube::instance()->get<InnertubeEndpoints::AccountMenu>();
    connect(reply, &InnertubeReply<InnertubeEndpoints::AccountMenu>::finished, this, [this](const InnertubeEndpoints::AccountMenu& endpoint)
    {
        qtTubeApp->creds().updateAccount(endpoint);
        const InnertubeObjects::ResponsiveImage& accountPhoto = endpoint.response.header.accountPhoto;
        if (const InnertubeObjects::GenericThumbnail* recAvatar = accountPhoto.recommendedQuality(avatarButton->size()))
            avatarButton->setImage(recAvatar->url, TubeLabel::Cached | TubeLabel::Rounded);
    });
}

void TopBar::setUpNotifications()
{
    scaleAppropriately();
    notificationBell->setVisible(InnerTube::instance()->hasAuthenticated());
    if (InnerTube::instance()->hasAuthenticated())
        updateNotificationCount();
}

void TopBar::showSettings()
{
    SettingsForm* settings = new SettingsForm;
    settings->show();
}

void TopBar::signOut()
{
    InnerTube::instance()->unauthenticate();
    setUpNotifications();
    avatarButton->hide();
    signInButton->show();
    emit signInStatusChanged();
    qtTubeApp->creds().clear();
}

void TopBar::trySignIn()
{
#ifndef INNERTUBE_NO_WEBENGINE
    InnerTube::instance()->authenticate();
#else
    QMessageBox::StandardButton box = QMessageBox::information(nullptr, "YouTube Login",
R"(
Could not bring up the YouTube login page because the Qt web engine is not available.
You will need to provide authentication credentials manually to log in.
For info on how to do this, see https://github.com/BowDown097/QtTube/wiki/Manually-getting-login-credentials.
)");
    if (box != QMessageBox::StandardButton::Ok)
        return;

    int index = CredentialsStore::instance()->getActiveLoginIndex();
    if (index == -1)
        index = 0;

    CredentialsStore::instance()->populateAuthStore(index);
    if (!InnerTube::instance()->hasAuthenticated())
    {
        QMessageBox::information(nullptr, "Not Logged In",
R"(
You didn't provide authentication credentials or the credentials you provided are invalid.
If you provided credentials, please check them, refer back to the previous linked guide if needed, and try again.
)");
        return;
    }

    postSignInSetup();
#endif
}

void TopBar::updateNotificationCount(int value)
{
    if (value >= 0) // if value is non-negative (default value is -1)
    {
        notificationBell->updatePixmap(value > 0, palette());
        notificationBell->updateCount(value);
    }
    else
    {
        auto reply = InnerTube::instance()->get<InnertubeEndpoints::UnseenCount>();
        connect(reply, &InnertubeReply<InnertubeEndpoints::UnseenCount>::finished, this, [this](const InnertubeEndpoints::UnseenCount& endpoint)
        {
            notificationBell->updatePixmap(endpoint.unseenCount > 0, palette());
            notificationBell->updateCount(endpoint.unseenCount);
        });
    }
}

void TopBar::updatePalette(const QPalette& palette)
{
    setPalette(palette);
    logo->setPixmap(UIUtils::iconThemed("qttube-full", palette).pixmap(logo->size()));
    notificationBell->updatePixmap(notificationBell->count->isVisible(), palette);
    searchBox->updatePalette(palette);
    settingsButton->setPixmap(UIUtils::pixmapThemed("settings", palette));

    if (!InnerTube::instance()->hasAuthenticated())
        notificationBell->hide();
}
