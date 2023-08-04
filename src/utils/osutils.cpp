#include "osutils.h"
#include <QDebug>
#include <QString>

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

namespace OSUtils
{
    void toggleIdleSleep(bool toggle)
    {
        const QString status = toggle ? "enable" : "disable";
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

#ifdef Q_OS_WIN
    void applyStringProp(HWND hWnd, LPCWSTR lpString, WORD property)
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

    void allowDarkModeForWindow(HWND hWnd, BOOL enable)
    {
        if (hWnd)
            applyStringProp(hWnd, enable ? L"Enabled" : NULL, 0xA91E);
        return;
    }

# if !_MSC_VER // inline assembly is unsupported on MSVC, so for the sake of simplicity, win 8 code is stripped out
    template<int syscall_id, typename... arglist>
    __attribute__((naked)) uint32_t __fastcall WinSyscall([[maybe_unused]] arglist... args)
    {
        asm volatile("mov %%rcx, %%r10; movl %0, %%eax; syscall; ret" :: "i"(syscall_id));
    }

    bool isWin8_0()
    {
        HMODULE hKern32 = GetModuleHandleW(L"kernel32.dll");
        return GetProcAddress(hKern32, "CreateFile2") != NULL //  Export added in 6.2 (8)
               && GetProcAddress(hKern32, "AppXFreeMemory") != NULL;  // Export added in 6.2 (8), removed in 6.3 (8.1)
    }

    bool isWin8_1()
    {
        HMODULE hKern32 = GetModuleHandleW(L"kernel32.dll");
        return GetProcAddress(hKern32, "CalloutOnFiberStack") != NULL //  Export added in 6.3 (8.1), Removed in 10.0.10586
               && GetProcAddress(hKern32, "SetThreadSelectedCpuSets") == NULL; // Export added in 10.0 (10)
    }
# endif

    bool isWin10()
    {
        HMODULE hKern32 = GetModuleHandleW(L"kernel32.dll");
        HMODULE hNtuser = GetModuleHandleW(L"ntdll.dll");
        return GetProcAddress(hKern32, "SetThreadSelectedCpuSets") != NULL
               && GetProcAddress(hNtuser, "ZwSetInformationCpuPartition") == NULL;
    }

    bool isWin11()
    {
        HMODULE hKern32 = GetModuleHandleW(L"kernel32.dll");
        return GetProcAddress(hKern32, "Wow64SetThreadDefaultGuestMachine") != NULL; // Win11 21h2+
    }

    void setWinDarkModeEnabled(WId winid, bool enabled)
    {
        HWND hWnd = reinterpret_cast<HWND>(winid);
        allowDarkModeForWindow(hWnd, enabled);

        BOOL darkEnabled = (BOOL)enabled;
        WINDOWCOMPOSITIONATTRIBDATA data = {
            WCA_USEDARKMODECOLORS,
            &darkEnabled,
            sizeof(darkEnabled)
        };

# if !_MSC_VER
        constexpr int NtUserSetWindowCompositionAttribute_NT6_2 = 0x13b4;
        constexpr int NtUserSetWindowCompositionAttribute_NT6_3 = 0x13e5;
# endif

        if (isWin10() || isWin11())
        {
            ((fnSetWindowCompositionAttribute)(PVOID)GetProcAddress(GetModuleHandleW(L"user32.dll"), "SetWindowCompositionAttribute"))
                (hWnd, &data);
            ((fnSetPreferredAppMode)(PVOID)GetProcAddress(GetModuleHandleW(L"uxtheme.dll"), MAKEINTRESOURCEA(135)))
                (AppMode_AllowDark);
        }
# if !_MSC_VER
        else if (isWin8_1())
            WinSyscall<NtUserSetWindowCompositionAttribute_NT6_3>(hWnd, &data);
        else if (isWin8_0())
            WinSyscall<NtUserSetWindowCompositionAttribute_NT6_2>(hWnd, &data);
# endif
    }
#endif
}
