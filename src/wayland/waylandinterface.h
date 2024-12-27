#pragma once

class QWindow;
struct wl_surface;
struct zwp_idle_inhibit_manager_v1;
struct zwp_idle_inhibitor_v1;

class WaylandInterface
{
public:
    enum InhibitIdleResult
    {
        INHIBIT_SUCCESS,
        INHIBIT_FAILURE,
        INHIBIT_SURFACE_NOT_FOUND,
        INHIBIT_MANAGER_NOT_FOUND
    };

    ~WaylandInterface();

    InhibitIdleResult inhibitIdle(bool inhibit);
    void initialize(QWindow* windowHandle);
    void setInhibitManager(zwp_idle_inhibit_manager_v1* inhibitManager) { m_inhibitManager = inhibitManager; }
private:
    zwp_idle_inhibit_manager_v1* m_inhibitManager{};
    zwp_idle_inhibitor_v1* m_inhibitor{};
    wl_surface* m_surface{};
};
