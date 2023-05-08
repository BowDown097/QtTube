#ifndef OSUTILITIES_H
#define OSUTILITIES_H

#ifdef Q_OS_MACOS
#include <IOKit/pwr_mgt/IOPMLib.h>
#endif

#ifdef Q_OS_WIN
#include <Windows.h>
#include <dwmapi.h>
#endif

// Windows-related code skidded from https://github.com/PolyMC/PolyMC/blob/develop/launcher/ui/WinDarkmode.cpp
class OSUtilities
{
public:
#ifdef Q_OS_WIN
    static bool isWin8_0();
    static bool isWin8_1();
    static bool isWin10();
    static bool isWin11();
    static void setWinDarkModeEnabled(WId winid, bool enabled);
#endif
    static void toggleIdleSleep(bool toggle);
#ifdef Q_OS_MACOS
private:
    inline static IOPMAssertionID sleepAssert;
#endif
#ifdef Q_OS_WIN
private:
    enum PreferredAppMode {
        Default,
        AllowDark,
        ForceDark,
        ForceLight,
        Max
    };

    enum WINDOWCOMPOSITIONATTRIB {
        WCA_UNDEFINED = 0,
        WCA_NCRENDERING_ENABLED = 1,
        WCA_NCRENDERING_POLICY = 2,
        WCA_TRANSITIONS_FORCEDISABLED = 3,
        WCA_ALLOW_NCPAINT = 4,
        WCA_CAPTION_BUTTON_BOUNDS = 5,
        WCA_NONCLIENT_RTL_LAYOUT = 6,
        WCA_FORCE_ICONIC_REPRESENTATION = 7,
        WCA_EXTENDED_FRAME_BOUNDS = 8,
        WCA_HAS_ICONIC_BITMAP = 9,
        WCA_THEME_ATTRIBUTES = 10,
        WCA_NCRENDERING_EXILED = 11,
        WCA_NCADORNMENTINFO = 12,
        WCA_EXCLUDED_FROM_LIVEPREVIEW = 13,
        WCA_VIDEO_OVERLAY_ACTIVE = 14,
        WCA_FORCE_ACTIVEWINDOW_APPEARANCE = 15,
        WCA_DISALLOW_PEEK = 16,
        WCA_CLOAK = 17,
        WCA_CLOAKED = 18,
        WCA_ACCENT_POLICY = 19,
        WCA_FREEZE_REPRESENTATION = 20,
        WCA_EVER_UNCLOAKED = 21,
        WCA_VISUAL_OWNER = 22,
        WCA_HOLOGRAPHIC = 23,
        WCA_EXCLUDED_FROM_DDA = 24,
        WCA_PASSIVEUPDATEMODE = 25,
        WCA_USEDARKMODECOLORS = 26,
        WCA_LAST = 27
    };

    struct WINDOWCOMPOSITIONATTRIBDATA {
        WINDOWCOMPOSITIONATTRIB Attrib;
        PVOID pvData;
        SIZE_T cbData;
    };

    using fnAllowDarkModeForWindow = BOOL (WINAPI *)(HWND hWnd, BOOL allow);
    using fnSetPreferredAppMode = PreferredAppMode (WINAPI *)(PreferredAppMode appMode);
    using fnSetWindowCompositionAttribute = BOOL (WINAPI *)(HWND hwnd, WINDOWCOMPOSITIONATTRIBDATA *);

    static template<int syscall_id, typename... arglist> __attribute((naked)) uint32_t __fastcall WinSyscall([[maybe_unused]] arglist... args)
    {
        asm volatile("mov %%rcx, %%r10; movl %0, %%eax; syscall; ret" :: "i"(syscall_id));
    }

    static void allowDarkModeForWindow(HWND hWnd, BOOL enable);
    static void applyStringProp(HWND hWnd, LPCWSTR lpString, WORD property);
#endif
};

#endif // OSUTILITIES_H
