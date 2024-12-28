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

#ifdef QTTUBE_HAS_WAYLAND
#include "qttubeapplication.h"
#endif

#ifdef Q_OS_MACOS
#include <IOKit/pwr_mgt/IOPMLib.h>
#endif

#ifdef Q_OS_WIN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace OSUtils
{
    void suspendIdleSleep(bool suspend)
    {
        const char* status = suspend ? "suspend" : "resume";
        #if defined(Q_OS_UNIX) && !defined(__APPLE__) && !defined(__MACH__)
        # ifdef QTTUBE_HAS_WAYLAND
        if (qApp->platformName() == "wayland")
        {
            WaylandInterface::InhibitIdleResult res = qtTubeApp->waylandInterface().inhibitIdle(suspend);
            if (res == WaylandInterface::INHIBIT_MANAGER_NOT_FOUND)
                qDebug().noquote() << "Failed to" << status << "idle sleep timer: Compositor doesn't support zwp_idle_inhibit_manager_v1 protocol.";
            else if (res == WaylandInterface::INHIBIT_SURFACE_NOT_FOUND)
                qDebug().noquote() << "Failed to" << status << "idle sleep timer: Could not get window surface.";
            else if (res == WaylandInterface::INHIBIT_FAILURE)
                qDebug().noquote() << "Failed to" << status << "idle sleep timer: Unknown failure.";
            return;
        }
        # endif // Wayland check
        # ifdef QTTUBE_HAS_XSS
        if (qApp->platformName() != "xcb")
        {
            qDebug().noquote() << "Failed to" << status << "idle sleep timer: Can only control idle sleep on X11 on Unix systems. Screen may sleep while watching videos.";
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
            qDebug().noquote() << "Failed to" << status << "idle sleep timer: Failed to get X11 display.";
            return;
        }

        int event, error, major, minor;
        if (XScreenSaverQueryExtension(display, &event, &error) != 1)
        {
            qDebug().noquote() << "Failed to" << status << "idle sleep timer: XScreenSaverQueryExtension failed.";
            return;
        }
        if (XScreenSaverQueryVersion(display, &major, &minor) != 1 || major < 1 || (major == 1 && minor < 1))
        {
            qDebug().noquote() << "Failed to" << status << "idle sleep timer: XScreenSaverQueryVersion failed.";
            return;
        }

        XScreenSaverSuspend(display, suspend);
        # else
        if (qApp->platformName() == "xcb")
            qDebug().noquote() << "Failed to" << status << "idle sleep timer: XScreenSaver support is not enabled in this build.";
        # endif // XScreenSaver check
        #elif defined(Q_OS_WIN)
        if (SetThreadExecutionState(suspend ? ES_DISPLAY_REQUIRED | ES_CONTINUOUS | ES_SYSTEM_REQUIRED : ES_CONTINUOUS) == NULL)
            qDebug().noquote() << "Failed to" << status << "idle sleep timer: SetThreadExecutionState failed.";
        #elif defined(Q_OS_MACOS)
        static IOPMAssertionID sleepAssert;

        if (!suspend && sleepAssert)
        {
            IOPMAssertionRelease(sleepAssert);
            return;
        }

        CFStringRef reason = CFSTR("QtTube video playing");
        IOReturn success = IOPMAssertionCreateWithName(kIOPMAssertionTypeNoDisplaySleep, kIOPMAssertionLevelOn, reason, &sleepAssert);
        if (success != kIOReturnSuccess)
            qDebug().noquote() << "Failed to" << status << "idle sleep timer: Creating IOPM assertion failed.";
        #endif // OS checks
    }
}
