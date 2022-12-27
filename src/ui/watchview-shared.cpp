#include "watchview-shared.h"
#include "http.h"
#include "../settingsstore.h"

#if defined(Q_OS_UNIX) && !defined(__APPLE__) && !defined(__MACH__)
#include <QApplication>
#include <X11/extensions/scrnsaver.h>
#elif defined (Q_OS_WIN)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

QSize WatchViewShared::calcPlayerSize(int widgetWidth, int windowHeight)
{
    int playerWidth = widgetWidth;
    int playerHeight = playerWidth * 9/16;
    if (playerHeight > windowHeight - 125)
    {
        playerHeight = windowHeight - 125;
        playerWidth = playerHeight * 16/9;
    }

    return QSize(playerWidth, playerHeight);
}

void WatchViewShared::setChannelIcon(const HttpReply& reply, ClickableLabel* channelIcon)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    channelIcon->setPixmap(pixmap.scaled(48, 48, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void WatchViewShared::setSubscriberCount(const InnertubeObjects::VideoSecondaryInfo& secondaryInfo, QLabel* subscribersLabel)
{
    if (!SettingsStore::instance().fullSubs)
    {
        subscribersLabel->setText(secondaryInfo.subscriberCountText.text);
        return;
    }

    Http http;
    http.setReadTimeout(2000);
    http.setMaxRetries(5);

    // have to catch errors here because this API really, REALLY likes to stop working
    HttpReply* reply = http.get(QUrl("https://api.socialcounts.org/youtube-live-subscriber-count/" + secondaryInfo.channelId));
    QObject::connect(reply, &HttpReply::error, [secondaryInfo, subscribersLabel] { subscribersLabel->setText(secondaryInfo.subscriberCountText.text); });
    QObject::connect(reply, &HttpReply::finished, [subscribersLabel](const HttpReply& reply) {
        int subs = QJsonDocument::fromJson(reply.body())["est_sub"].toInt();
        subscribersLabel->setText(QLocale::system().toString(subs) + " subscribers");
    });
}

void WatchViewShared::toggleIdleSleep(bool toggle)
{
#if defined(Q_OS_UNIX) && !defined(__APPLE__) && !defined(__MACH__)
    Display* display = qApp->nativeInterface<QNativeInterface::QX11Application>()->display();
    if (!display)
    {
        qDebug() << "Failed to toggle idle sleep: Failed to get X11 display";
        return;
    }

    int event, error, major, minor;
    if (XScreenSaverQueryExtension(display, &event, &error) != 1)
    {
        qDebug() << "Failed to toggle idle sleep: XScreenSaverQueryExtension failed";
        return;
    }
    if (XScreenSaverQueryVersion(display, &major, &minor) != 1 || major < 1 || (major == 1 && minor < 1))
    {
        qDebug() << "Failed to toggle idle sleep: XScreenSaverQueryVersion failed or definitely returned the wrong version";
        return;
    }

    XScreenSaverSuspend(display, toggle);
#elif defined(Q_OS_WIN)
    if (SetThreadExecutionState(toggle ? ES_DISPLAY_REQUIRED | ES_CONTINUOUS | ES_SYSTEM_REQUIRED : ES_CONTINUOUS) == NULL)
        qDebug() << "Failed to toggle idle sleep: SetThreadExecutionState failed";
#elif defined(Q_OS_MACOS)
    if (!toggle && sleepAssert)
    {
        IOPMAssertionRelease(sleepAssert);
        return;
    }

    CFStringRef reason = CFSTR("QtTube video playing");
    IOReturn success = IOPMAssertionCreateWithName(kIOPMAssertionTypeNoDisplaySleep, kIOPMAssertionLevelOn, reason, &sleepAssert);
    if (success != kIOReturnSuccess)
        qDebug() << "Failed to toggle idle sleep: Creating IOPM assertion failed";
#else
    qDebug() << "Failed to toggle idle sleep: Unsupported OS";
#endif
}
