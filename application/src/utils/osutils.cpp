#include "osutils.h"
#include <QDir>
#include <QFileInfo>

#ifdef QTTUBE_HAS_WAYLAND
#include "qttubeapplication.h"
#endif

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
    QString getFullPath(const QFileInfo& fileInfo)
    {
        QString absoluteFilePath = fileInfo.absoluteFilePath();
        if (fileInfo.exists())
            return absoluteFilePath;

    #ifdef Q_OS_WIN
        absoluteFilePath += ".exe";
        if (QFile::exists(absoluteFilePath))
            return absoluteFilePath;
    #endif

        if (QString pathEnv = qgetenv("PATH"); !pathEnv.isEmpty())
        {
            const QStringList pathParts = pathEnv.split(QDir::listSeparator());
            for (const QString& dir : pathParts)
            {
                QString fullPath = dir + QDir::separator() + fileInfo.fileName();
                if (QFile::exists(fullPath))
                    return fullPath;

            #ifdef Q_OS_WIN
                fullPath += ".exe";
                if (QFile::exists(fullPath))
                    return fullPath;
            #endif
            }
        }

        return QString();
    }

#ifdef Q_OS_MACOS
    void suspendIdleSleepMacOS(bool suspend, const char* status)
    {
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
    }
#endif

#ifdef QTTUBE_HAS_WAYLAND
    void suspendIdleSleepWayland(bool suspend, const char* status)
    {
        WaylandInterface::InhibitIdleResult res = qtTubeApp->waylandInterface().inhibitIdle(suspend);
        if (res == WaylandInterface::INHIBIT_MANAGER_NOT_FOUND)
            qDebug().noquote() << "Failed to" << status << "idle sleep timer: Compositor doesn't support zwp_idle_inhibit_manager_v1 protocol.";
        else if (res == WaylandInterface::INHIBIT_SURFACE_NOT_FOUND)
            qDebug().noquote() << "Failed to" << status << "idle sleep timer: Could not get window surface.";
        else if (res == WaylandInterface::INHIBIT_FAILURE)
            qDebug().noquote() << "Failed to" << status << "idle sleep timer: Unknown failure.";
    }
#endif

#ifdef Q_OS_WIN
    void suspendIdleSleepWindows(bool suspend, const char* status)
    {
        if (!SetThreadExecutionState(suspend ? ES_DISPLAY_REQUIRED | ES_CONTINUOUS : ES_CONTINUOUS))
            qDebug().noquote() << "Failed to" << status << "idle sleep timer: SetThreadExecutionState failed.";
    }
#endif

#ifdef QTTUBE_HAS_XSS
    void suspendIdleSleepX11(bool suspend, const char* status)
    {
    #if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
        Display* display = qApp->nativeInterface<QNativeInterface::QX11Application>()->display();
    #elif defined(QTTUBE_HAS_X11EXTRAS)
        Display* display = QX11Info::display();
    #else
        Display* display = XOpenDisplay(nullptr); // last resort!
    #endif

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
    }
#endif

    void suspendIdleSleep(bool suspend)
    {
        const char* status = suspend ? "suspend" : "resume";
    #if defined(Q_OS_UNIX) && !defined(__APPLE__) && !defined(__MACH__)
        if (qApp->platformName() == "wayland")
        {
        #ifdef QTTUBE_HAS_WAYLAND
            suspendIdleSleepWayland(suspend, status);
        #else
            qDebug().noquote() << "Failed to" << status << "idle sleep timer: Wayland protocol support is not enabled in this build.";
        #endif
        }
        else if (qApp->platformName() == "xcb")
        {
        #ifdef QTTUBE_HAS_XSS
            suspendIdleSleepX11(suspend, status);
        #else
            qDebug().noquote() << "Failed to" << status << "idle sleep timer: XScreenSaver support is not enabled in this build.";
        #endif
        }
        else
        {
            qDebug().noquote() << "Failed to" << status << "idle sleep timer: Unsupported display server (not X11 or Wayland).";
        }
    #elif defined(Q_OS_WIN)
        suspendIdleSleepWindows(suspend, status);
    #elif defined(Q_OS_MACOS)
        suspendIdleSleepMacOS(suspend, status);
    #else
        qDebug().noquote() << "Failed to" << status << "idle sleep timer: Unsupported operating system.";
    #endif
    }
}
