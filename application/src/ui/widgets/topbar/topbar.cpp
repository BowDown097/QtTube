#include "topbar.h"
#include "qttubeapplication.h"
#include "ui/forms/settings/settingsform.h"
#include "utils/uiutils.h"
#include <QApplication>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QTabBar>

inline bool hasAuthenticated()
{
    const PluginData* plugin = qtTubeApp->plugins().activePlugin();
    return plugin && plugin->auth && !plugin->auth->isEmpty();
}

TopBar::TopBar(QWidget* parent)
    : QWidget(parent),
      animation(new QPropertyAnimation(this, "geometry", this)),
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

void TopBar::postSignInSetup()
{
    if (const PluginData* plugin = qtTubeApp->plugins().activePlugin())
    {
        if (QtTubePlugin::AccountReply* reply = plugin->interface->getActiveAccount())
        {
            connect(reply, &QtTubePlugin::AccountReply::exception, this, [this](const QtTubePlugin::Exception& ex) {
                QMessageBox::warning(nullptr, "Failed to get active account data", ex.message());
            });
            connect(reply, &QtTubePlugin::AccountReply::finished, this, [this, plugin](const QtTubePlugin::InitialAccountData& data) {
                updateNotificationCount(data.notificationCount);
                avatarButton->setImage(data.avatarUrl, TubeLabel::Cached | TubeLabel::Rounded);
                plugin->auth->update(data);
                scaleAppropriately();
            });
        }
        else
        {
            QMessageBox::critical(nullptr, "Failed to get active account data", "No method has been provided. Not proceeding with authentication.");
            return;
        }
    }

    updateUIForSignInState(true);
    emit signInStatusChanged();
}

void TopBar::scaleAppropriately()
{
    if (hasAuthenticated())
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

void TopBar::showSettings()
{
    SettingsForm* settings = new SettingsForm;
    settings->show();
}

void TopBar::signOut()
{
    if (const PluginData* plugin = qtTubeApp->plugins().activePlugin(); plugin && plugin->auth)
        plugin->auth->clear();
    updateUIForSignInState(false);
    emit signInStatusChanged();
}

void TopBar::trySignIn()
{
    if (const PluginData* plugin = qtTubeApp->plugins().activePlugin(); plugin && plugin->auth)
        plugin->auth->startAuthRoutine();
}

void TopBar::updateNotificationCount(int value)
{
    notificationBell->updatePixmap(value > 0, palette());
    notificationBell->updateCount(value);
}

void TopBar::updatePalette(const QPalette& palette)
{
    setPalette(palette);
    logo->setPixmap(UIUtils::iconThemed("qttube-full", palette).pixmap(logo->size()));
    notificationBell->updatePixmap(notificationBell->count->isVisible(), palette);
    searchBox->updatePalette(palette);
    settingsButton->setPixmap(UIUtils::pixmapThemed("settings", palette));

    if (!hasAuthenticated())
        notificationBell->hide();
}

void TopBar::updateUIForSignInState(bool signedIn)
{
    if (signedIn)
    {
        avatarButton->show();
        signInButton->hide();
    }
    else
    {
        avatarButton->hide();
        notificationBell->hide();
        signInButton->show();
    }

    scaleAppropriately();
}
