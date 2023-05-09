#include "osutilities.h"

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
static IOPMAssertionID sleepAssert;
#endif

#ifdef Q_OS_WIN
static constexpr std::array<byte, 4> NtUserSetWindowCompositionAttribute_NT6_2 = { 0xB4, 0x13, 0x00, 0x00 };
static constexpr std::array<byte, 4> NtUserSetWindowCompositionAttribute_NT6_3 = { 0xE5, 0x13, 0x00, 0x00 };
#endif

namespace OSUtilities
{
    void toggleIdleSleep(bool toggle)
    {
        const char* statusString = toggle ? "enable" : "disable";
        #if defined(Q_OS_UNIX) && !defined(__APPLE__) && !defined(__MACH__)
            #ifdef QTTUBE_HAS_XSS
                if (qApp->platformName() != "xcb")
                {
                    qDebug().noquote() << "Failed to" << statusString << "idle sleep: Can only toggle sleep on X11 on Unix systems. Screen may sleep while watching videos.";
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
                    qDebug().noquote() << "Failed to" << statusString << "idle sleep: Failed to get X11 display";
                    return;
                }

                int event, error, major, minor;
                if (XScreenSaverQueryExtension(display, &event, &error) != 1)
                {
                    qDebug().noquote() << "Failed to" << statusString << "idle sleep: XScreenSaverQueryExtension failed";
                    return;
                }
                if (XScreenSaverQueryVersion(display, &major, &minor) != 1 || major < 1 || (major == 1 && minor < 1))
                {
                    qDebug().noquote() << "Failed to" << statusString << "idle sleep: XScreenSaverQueryVersion failed";
                    return;
                }

                XScreenSaverSuspend(display, toggle);
            #else
                qDebug().noquote() << "Failed to" << statusString << "idle sleep: XScreenSaver support is not enabled in this build";
            #endif // XScreenSaver check
        #elif defined(Q_OS_WIN)
            if (SetThreadExecutionState(toggle ? ES_DISPLAY_REQUIRED | ES_CONTINUOUS | ES_SYSTEM_REQUIRED : ES_CONTINUOUS) == NULL)
                qDebug().noquote() << "Failed to" << statusString << "idle sleep: SetThreadExecutionState failed";
        #elif defined(Q_OS_MACOS)
            if (!toggle && sleepAssert)
            {
                IOPMAssertionRelease(sleepAssert);
                return;
            }

            CFStringRef reason = CFSTR("QtTube video playing");
            IOReturn success = IOPMAssertionCreateWithName(kIOPMAssertionTypeNoDisplaySleep, kIOPMAssertionLevelOn, reason, &sleepAssert);
            if (success != kIOReturnSuccess)
                qDebug().noquote() << "Failed to" << statusString << "idle sleep: Creating IOPM assertion failed";
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

    template<std::array<BYTE, 4> syscall_id, typename... arglist> void __fastcall WinSyscall([[maybe_unused]] arglist... args)
    {
        BYTE pCode[] =
        {
            0x4C, 0x8B, 0xD1, // mov rcx, r10
            0xB8, syscall_id[0], syscall_id[1], syscall_id[2], syscall_id[3], // mov syscall_id, eax
            0x0F, 0x05, // syscall
            0xC3 // ret
        };

        LPVOID buf = VirtualAlloc(nullptr, sizeof(pCode), MEM_COMMIT, PAGE_READWRITE);
        memcpy(buf, pCode, sizeof(pCode));

        DWORD oldProtect;
        VirtualProtect(buf, sizeof(pCode), PAGE_EXECUTE_READWRITE, &oldProtect);

        reinterpret_cast<void(*)()>(buf)();
        VirtualFree(buf, 0, MEM_RELEASE);
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

        if (isWin10() || isWin11())
        {
            ((fnSetWindowCompositionAttribute)(PVOID)GetProcAddress(GetModuleHandleW(L"user32.dll"), "SetWindowCompositionAttribute"))
                (hWnd, &data);
            ((fnSetPreferredAppMode)(PVOID)GetProcAddress(GetModuleHandleW(L"uxtheme.dll"), MAKEINTRESOURCEA(135)))
                (AppMode_AllowDark);
        }
        else if (isWin8_1())
            WinSyscall<NtUserSetWindowCompositionAttribute_NT6_3>(hWnd, &data);
        else if (isWin8_0())
            WinSyscall<NtUserSetWindowCompositionAttribute_NT6_2>(hWnd, &data);
    }
#endif
}
