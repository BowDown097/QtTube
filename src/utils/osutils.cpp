#include "osutils.h"
#include <QDebug>

#if defined(Q_OS_UNIX) && !defined(__APPLE__) && !defined(__MACH__)
# include <QApplication>
# ifdef QTTUBE_HAS_XSS
#  include <X11/extensions/scrnsaver.h>
#  ifdef QTTUBE_HAS_X11EXTRAS
#   include <QtX11Extras/QX11Info>
#  endif // X11Extras check
# endif // XScreenSaver check
#endif // Non-Mac Unix check

#ifdef Q_OS_MACOS
#include <IOKit/pwr_mgt/IOPMLib.h>
#endif

#ifdef Q_OS_WIN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace OSUtils
{
    void toggleIdleSleep(bool toggle)
    {
        const char* status = toggle ? "enable" : "disable";
        #if defined(Q_OS_UNIX) && !defined(__APPLE__) && !defined(__MACH__)
        # ifdef QTTUBE_HAS_XSS
        if (qApp->platformName() != "xcb")
        {
            qDebug().noquote() << "Failed to" << status << "idle sleep: Can only toggle sleep on X11 on Unix systems. Screen may sleep while watching videos.";
            return;
        }

        #  if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            Display* display = qApp->nativeInterface<QNativeInterface::QX11Application>()->display();
        #  elif defined(QTTUBE_HAS_X11EXTRAS)
            Display* display = QX11Info::display();
        #  else
            Display* display = XOpenDisplay(NULL); // last resort!
        #  endif // X11 display getter

        if (!display)
        {
            qDebug().noquote() << "Failed to" << status << "idle sleep: Failed to get X11 display";
            return;
        }

        int event, error, major, minor;
        if (XScreenSaverQueryExtension(display, &event, &error) != 1)
        {
            qDebug().noquote() << "Failed to" << status << "idle sleep: XScreenSaverQueryExtension failed";
            return;
        }
        if (XScreenSaverQueryVersion(display, &major, &minor) != 1 || major < 1 || (major == 1 && minor < 1))
        {
            qDebug().noquote() << "Failed to" << status << "idle sleep: XScreenSaverQueryVersion failed";
            return;
        }

        XScreenSaverSuspend(display, toggle);
        # else
            qDebug().noquote() << "Failed to" << status << "idle sleep: XScreenSaver support is not enabled in this build";
        # endif // XScreenSaver check
        #elif defined(Q_OS_WIN)
        if (SetThreadExecutionState(toggle ? ES_DISPLAY_REQUIRED | ES_CONTINUOUS | ES_SYSTEM_REQUIRED : ES_CONTINUOUS) == NULL)
            qDebug().noquote() << "Failed to" << status << "idle sleep: SetThreadExecutionState failed";
        #elif defined(Q_OS_MACOS)
        static IOPMAssertionID sleepAssert;

        if (!toggle && sleepAssert)
        {
            IOPMAssertionRelease(sleepAssert);
            return;
        }

        CFStringRef reason = CFSTR("QtTube video playing");
        IOReturn success = IOPMAssertionCreateWithName(kIOPMAssertionTypeNoDisplaySleep, kIOPMAssertionLevelOn, reason, &sleepAssert);
        if (success != kIOReturnSuccess)
            qDebug().noquote() << "Failed to" << status << "idle sleep: Creating IOPM assertion failed";
        #endif // OS checks
    }
}
