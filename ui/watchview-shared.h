#ifndef WATCHVIEWSHARED_H
#define WATCHVIEWSHARED_H
#if defined(Q_OS_UNIX) && !defined(__APPLE__) && !defined(__MACH__)
#include <QApplication>
#include <X11/extensions/scrnsaver.h>
#elif defined (Q_OS_WIN)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#elif defined (Q_OS_MACOS)
#include <IOKit/pwr_mgt/IOPMLib.h>
#endif

class WatchViewShared
{
public:
#ifdef Q_OS_MACOS
    inline static IOPMAssertionID sleepAssert;
#endif
    static void toggleIdleSleep(bool disable)
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

        XScreenSaverSuspend(display, disable);
    #elif defined(Q_OS_WIN)
        if (SetThreadExecutionState(disable ? ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_AWAYMODE_REQUIRED : ES_CONTINUOUS) == NULL)
            qDebug() << "Failed to toggle idle sleep: SetThreadExecutionState failed";
    #elif defined(Q_OS_MACOS)
        if (!disable && sleepAssert)
        {
            IOPMAssertionRelease(sleepAssert);
            return;
        }

        CFStringRef* reason = CFSTR("QtTube video playing");
        IOReturn success = IOPMAssertionCreateWithName(kIOPMAssertionTypeNoDisplaySleep, kIOPMAssertionLevelOn, reason, &sleepAssert);
        if (success != kIOReturnSuccess)
            qDebug() << "Failed to toggle idle sleep: Creating IOPM assertion failed";
    #else
        qDebug() << "Failed to toggle idle sleep: Unsupported OS";
    #endif
    }
};

#endif // WATCHVIEWSHARED_H
