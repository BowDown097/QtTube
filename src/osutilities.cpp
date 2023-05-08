#include "osutilities.h"
#include <QApplication>

#ifdef QTTUBE_HAS_XSS
#include <X11/extensions/scrnsaver.h>
#ifdef QTTUBE_HAS_X11EXTRAS
#include <QtX11Extras/QX11Info>
#endif // X11Extras check
#endif // XScreenSaver check

#ifdef Q_OS_WIN
QPair<int, int> OSUtilities::getWinVer()
{
    NTSTATUS(WINAPI *RtlGetVersion)(LPOSVERSIONINFOEXW);
    OSVERSIONINFOEXW osInfo;

    *(FARPROC*)&RtlGetVersion = GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion");
    if (RtlGetVersion != NULL)
    {
        osInfo.dwOSVersionInfoSize = sizeof(osInfo);
        RtlGetVersion(&osInfo);
    }

    return qMakePair(osInfo.dwMajorVersion, osInfo.dwMinorVersion);
}

void OSUtilities::setDarkWinTitlebar(WId winid, bool darkmode)
{
    HWND hwnd = reinterpret_cast<HWND>(winid);
    BOOL dark = (BOOL)darkmode;

    HMODULE hUxtheme = LoadLibraryExW(L"uxtheme.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
    HMODULE hUser32 = GetModuleHandleW(L"user32.dll");

    // For those confused how this works, dlls have export numbers but some can have no name and these have no name. So an address is gotten by export number. If this ever changes, it will break.
    fnAllowDarkModeForWindow AllowDarkModeForWindow = (fnAllowDarkModeForWindow)(PVOID)GetProcAddress(hUxtheme, MAKEINTRESOURCEA(133));
    fnSetPreferredAppMode SetPreferredAppMode = (fnSetPreferredAppMode)(PVOID)GetProcAddress(hUxtheme, MAKEINTRESOURCEA(135));
    fnSetWindowCompositionAttribute SetWindowCompositionAttribute = (fnSetWindowCompositionAttribute)(PVOID)GetProcAddress(hUser32, "SetWindowCompositionAttribute");

    SetPreferredAppMode(AllowDark);
    AllowDarkModeForWindow(hwnd, dark);
    WINDOWCOMPOSITIONATTRIBDATA data = {
        WCA_USEDARKMODECOLORS,
        &dark,
        sizeof(dark)
    };
    SetWindowCompositionAttribute(hwnd, &data);
}
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
