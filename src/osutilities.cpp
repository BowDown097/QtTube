#include "osutilities.h"
#include <QApplication>

#ifdef QTTUBE_HAS_XSS
#include <X11/extensions/scrnsaver.h>
#ifdef QTTUBE_HAS_X11EXTRAS
#include <QtX11Extras/QX11Info>
#endif // X11Extras check
#endif // XScreenSaver check

#ifdef Q_OS_WIN
void OSUtilities::allowDarkModeForWindow(HWND hWnd, BOOL enable)
{
    if (hWnd)
        applyStringProp(hWnd, enable ? L"Enabled" : NULL, 0xA91E);
    return;
}

void OSUtilities::applyStringProp(HWND hWnd, LPCWSTR lpString, WORD property)
{
    WORD prop = (uint16_t)(uint64_t)GetPropW(hWnd, (LPCWSTR)(uint64_t)property);
    if (prop)
    {
        DeleteAtom(prop);
        RemovePropW(hWnd, (LPCWSTR)(uint64_t)property);
    }
    if (lpString)
    {
        ATOM v = AddAtomW(lpString);
        if (v)
            SetPropW(hWnd, (LPCWSTR)(uint64_t)property, (HANDLE)(uint64_t)v);
    }
}

bool OSUtilities::isWin8_0()
{
    HMODULE hKern32 = GetModuleHandleW(L"kernel32.dll");
    return GetProcAddress(hKern32, "CreateFile2") != NULL //  Export added in 6.2 (8)
           && GetProcAddress(hKern32, "AppXFreeMemory") != NULL;  // Export added in 6.2 (8), removed in 6.3 (8.1)
}

bool OSUtilities::isWin8_1()
{
    HMODULE hKern32 = GetModuleHandleW(L"kernel32.dll");
    return GetProcAddress(hKern32, "CalloutOnFiberStack") != NULL //  Export added in 6.3 (8.1), Removed in 10.0.10586
           && GetProcAddress(hKern32, "SetThreadSelectedCpuSets") == NULL; // Export added in 10.0 (10)
}

bool OSUtilities::isWin10()
{
    HMODULE hKern32 = GetModuleHandleW(L"kernel32.dll");
    HMODULE hNtuser = GetModuleHandleW(L"ntdll.dll");
    return GetProcAddress(hKern32, "SetThreadSelectedCpuSets") != NULL
           && GetProcAddress(hNtuser, "ZwSetInformationCpuPartition") == NULL;
}

bool OSUtilities::isWin11()
{
    HMODULE hKern32 = GetModuleHandleW(L"kernel32.dll");
    return GetProcAddress(hKern32, "Wow64SetThreadDefaultGuestMachine") != NULL; // Win11 21h2+
}

void OSUtilities::setWinDarkModeEnabled(WId winid, bool enabled)
{
    HWND hWnd = reinterpret_cast<HWND>(winid);
    allowDarkModeForWindow(hWnd, enabled);

    BOOL darkEnabled = (BOOL)enabled;
    WINDOWCOMPOSITIONATTRIBDATA data = {
        WCA_USEDARKMODECOLORS,
        &darkEnabled,
        sizeof(darkEnabled)
    };

    constexpr int NtUserSetWindowCompositionAttribute_NT6_2 = 0x13b4;
    constexpr int NtUserSetWindowCompositionAttribute_NT6_3 = 0x13e5;

    if (isWin8_0())
        WinSyscall<NtUserSetWindowCompositionAttribute_NT6_2>(hWnd, &data);
    if (isWin8_1())
        WinSyscall<NtUserSetWindowCompositionAttribute_NT6_3>(hWnd, &data);
    if (isWin10() || isWin11())
    {
        ((fnSetWindowCompositionAttribute)(PVOID)GetProcAddress(GetModuleHandleW(L"user32.dll"), "SetWindowCompositionAttribute"))
            (hWnd, &data);
        ((fnSetPreferredAppMode)(PVOID)GetProcAddress(GetModuleHandleW(L"uxtheme.dll"), MAKEINTRESOURCEA(135)))
            (AppMode_AllowDark);
    }
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
