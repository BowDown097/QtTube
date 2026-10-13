#pragma once
#include <cstdint>

struct wl_registry;
struct wl_surface;
struct zwp_idle_inhibit_manager_v1;
struct zwp_idle_inhibitor_v1;

class WaylandInterface
{
public:
    enum class IdleInhibitResult
    {
        Success,
        Failure,
        SurfaceNotFound,
        ManagerNotFound
    };

    ~WaylandInterface();
    IdleInhibitResult inhibitIdle(bool inhibit);
    void initialize();
private:
    zwp_idle_inhibit_manager_v1* m_inhibitManager{};
    zwp_idle_inhibitor_v1* m_inhibitor{};
    wl_surface* m_surface{};

    friend void wl_global(
        void* data, wl_registry* registry, uint32_t name, const char* interface, uint32_t version);
};
