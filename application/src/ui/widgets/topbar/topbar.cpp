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

TopBar::TopBar(QWidget* parent)
    : QWidget(parent),
      avatarButton(new TubeLabel(this)),
      logo(new TubeLabel(this)),
      notificationBell(new TopBarBell(this)),
      searchBox(new SearchBox(this)),
      m_animation(new QPropertyAnimation(this, "geometry", this)),
      m_settingsButton(new TubeLabel(this)),
      m_signInButton(new QPushButton(this))
{
    resize(parent->width(), 35);
    setAutoFillBackground(true);

    m_animation->setDuration(250);
    m_animation->setEasingCurve(QEasingCurve::InOutQuint);

    logo->move(10, 2);
    logo->resize(134, 30);
    logo->setClickable(true);
    logo->setScaledContents(true);
    // for some reason, this is rather low-res when using just pixmapThemed...
    logo->setPixmap(UIUtils::iconThemed("qttube-full").pixmap(logo->size()));

    searchBox->move(logo->width() + logo->x() + 8, 0);
    searchBox->resize(475, 35);

    notificationBell->move(searchBox->width() + searchBox->x() + 8, 2);

    m_settingsButton->move(notificationBell->width() + notificationBell->x() + 8, 3);
    m_settingsButton->resize(30, 30);
    m_settingsButton->setClickable(true);
    m_settingsButton->setScaledContents(true);
    m_settingsButton->setPixmap(UIUtils::pixmapThemed("settings"));
    connect(m_settingsButton, &TubeLabel::clicked, this, &TopBar::showSettings);

    avatarButton->hide();
    avatarButton->move(searchBox->width() + searchBox->x() + 8, 3);
    avatarButton->resize(30, 30);
    avatarButton->setClickable(true);
    avatarButton->setScaledContents(true);

    m_signInButton->move(m_settingsButton->width() + m_settingsButton->x() + 8, 0);
    m_signInButton->resize(80, 35);
    m_signInButton->setText("Sign in");
    connect(m_signInButton, &QPushButton::clicked, this, &TopBar::trySignIn);
}

void TopBar::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::PaletteChange)
    {
        logo->setPixmap(UIUtils::iconThemed("qttube-full").pixmap(logo->size()));
        notificationBell->updatePixmap(notificationBell->hasNotifications());
        m_settingsButton->setPixmap(UIUtils::pixmapThemed("settings"));
    }

    QWidget::changeEvent(event);
}

void TopBar::handleMouseEvent(QMouseEvent* event)
{
    bool interferingWithTab{};
    if (QWidget* widgetAtPoint = qApp->widgetAt(QCursor::pos()))
        interferingWithTab = strncmp(widgetAtPoint->metaObject()->className(), "QTab", 4) == 0;
    if (m_alwaysShow || m_animation->state() == QAbstractAnimation::Running || interferingWithTab)
        return;

    if (event->pos().y() < height())
    {
        if (isHidden())
        {
            m_animation->setStartValue(QRect(0, 0, width(), 0));
            m_animation->setEndValue(QRect(0, 0, width(), height()));
            disconnect(m_animation, &QPropertyAnimation::finished, this, nullptr);
            m_animation->start();
            show();
        }
    }
    else if (isVisible())
    {
        m_animation->setStartValue(QRect(0, 0, width(), height()));
        m_animation->setEndValue(QRect(0, 0, width(), 0));
        connect(m_animation, &QPropertyAnimation::finished, this, [this] {
            hide();
            resize(width(), m_animation->startValue().toRect().height());
        });
        m_animation->start();
    }
}

void TopBar::postSignInSetup()
{
    if (const PluginData* plugin = qtTubeApp->plugins().activePlugin())
    {
        if (QtTubePlugin::AccountReply* reply = plugin->interface->getActiveAccount())
        {
            connect(reply, &QtTubePlugin::AccountReply::exception, this, [this](const QtTubePlugin::Exception& ex) {
                QMessageBox::warning(nullptr, "Failed to Load Account Data", ex.message());
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
            QMessageBox::critical(nullptr, "Failed to Load Account Data", "The active plugin has implemented authentication incompletely. The operation cannot continue.");
            return;
        }
    }

    updateUIForSignInState(true);
    emit signInStatusChanged();
}

void TopBar::scaleAppropriately()
{
    if (qtTubeApp->plugins().hasAuthenticated())
    {
        searchBox->resize(502 + width() - 800, 35);
        notificationBell->move(searchBox->width() + searchBox->x() + 8, 2);
        m_settingsButton->move(notificationBell->width() + notificationBell->x() + 8, 4);
        avatarButton->move(m_settingsButton->width() + m_settingsButton->x() + 8, 3);
    }
    else
    {
        searchBox->resize(452 + width() - 800, 35);
        notificationBell->move(searchBox->width() + searchBox->x() + 8, 2);
        m_settingsButton->move(notificationBell->width() + notificationBell->x() + 8, 4);
        m_signInButton->move(m_settingsButton->width() + m_settingsButton->x() + 8, 0);
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
    notificationBell->updatePixmap(value > 0);
    notificationBell->updateCount(value);
}

void TopBar::updateUIForSignInState(bool signedIn)
{
    if (signedIn)
    {
        avatarButton->show();
        m_signInButton->hide();
    }
    else
    {
        avatarButton->hide();
        m_signInButton->show();
    }

    scaleAppropriately();
}
