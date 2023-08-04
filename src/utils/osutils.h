#ifndef OSUTILS_H
#define OSUTILS_H
#include <qglobal.h>

#ifdef Q_OS_WIN
#include <dwmapi.h>
#include <qwindowdefs.h>
#include <windows.h>
#endif

// Windows-related code skidded from https://github.com/PolyMC/PolyMC/blob/develop/launcher/ui/WinDarkmode.cpp
namespace OSUtils
{
    void toggleIdleSleep(bool toggle);
#ifdef Q_OS_WIN
# if !_MSC_VER
    bool isWin8_0();
    bool isWin8_1();
# endif
    bool isWin10();
    bool isWin11();
    void setWinDarkModeEnabled(WId winid, bool enabled);

    enum PreferredAppMode
    {
        AppMode_Default,
        AppMode_AllowDark,
        AppMode_ForceDark,
        AppMode_ForceLight,
        AppMode_Max
    };

    enum WINDOWCOMPOSITIONATTRIB
    {
        WCA_UNDEFINED,
        WCA_NCRENDERING_ENABLED,
        WCA_NCRENDERING_POLICY,
        WCA_TRANSITIONS_FORCEDISABLED,
        WCA_ALLOW_NCPAINT,
        WCA_CAPTION_BUTTON_BOUNDS,
        WCA_NONCLIENT_RTL_LAYOUT,
        WCA_FORCE_ICONIC_REPRESENTATION,
        WCA_EXTENDED_FRAME_BOUNDS,
        WCA_HAS_ICONIC_BITMAP,
        WCA_THEME_ATTRIBUTES,
        WCA_NCRENDERING_EXILED,
        WCA_NCADORNMENTINFO,
        WCA_EXCLUDED_FROM_LIVEPREVIEW,
        WCA_VIDEO_OVERLAY_ACTIVE,
        WCA_FORCE_ACTIVEWINDOW_APPEARANCE,
        WCA_DISALLOW_PEEK,
        WCA_CLOAK,
        WCA_CLOAKED,
        WCA_ACCENT_POLICY,
        WCA_FREEZE_REPRESENTATION,
        WCA_EVER_UNCLOAKED,
        WCA_VISUAL_OWNER,
        WCA_HOLOGRAPHIC,
        WCA_EXCLUDED_FROM_DDA,
        WCA_PASSIVEUPDATEMODE,
        WCA_USEDARKMODECOLORS,
        WCA_LAST
    };

    struct WINDOWCOMPOSITIONATTRIBDATA
    {
        WINDOWCOMPOSITIONATTRIB Attrib;
        PVOID pvData;
        SIZE_T cbData;
    };

    using fnSetPreferredAppMode = PreferredAppMode (WINAPI *)(PreferredAppMode appMode);
    using fnSetWindowCompositionAttribute = BOOL (WINAPI *)(HWND hwnd, WINDOWCOMPOSITIONATTRIBDATA *);
#endif
}

#endif // OSUTILS_H
