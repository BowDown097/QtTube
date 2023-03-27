#include "osutilities.h"
#include <QApplication>

#ifdef QTTUBE_HAS_XSS
#include <X11/extensions/scrnsaver.h>
#ifdef QTTUBE_HAS_X11EXTRAS
#include <QtX11Extras/QX11Info>
#endif // X11Extras check
#endif // XScreenSaver check

#ifdef Q_OS_WIN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

void OSUtilities::toggleIdleSleep(bool toggle)
{
#if defined(Q_OS_UNIX) && !defined(__APPLE__) && !defined(__MACH__)
    #ifdef QTTUBE_HAS_XSS
        if (qApp->platformName() != "xcb")
        {
            qDebug() << "Failed to toggle idle sleep: Can only toggle sleep on X11 on Unix systems. Screen may sleep while watching videos.";
            return;
        }

        #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            Display* display = qApp->nativeInterface<QNativeInterface::QX11Application>()->display();
        #elif defined(QTTUBE_HAS_X11EXTRAS)
            Display* display = QX11Info::display();
        #else
            Display* display = XOpenDisplay(NULL); // last resort!
        #endif // X11 display getter

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
    #else
        qDebug() << "Failed to toggle idle sleep: XScreenSaver support is not enabled in this build";
        Q_UNUSED(toggle);
    #endif // XScreenSaver check
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
    Q_UNUSED(toggle);
#endif // OS checks
}
