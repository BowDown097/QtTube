#include "waylandinterface.hpp"
#include "wayland/ext-idle-inhibit-unstable-v1.h"
#include <QGuiApplication>

#if QT_VERSION < QT_VERSION_CHECK(6, 5, 0)
#include <QtGui/qpa/qplatformnativeinterface.h>
#endif

void wl_global(
    void* data, wl_registry* registry, uint32_t name, const char* interface, uint32_t version)
{
    auto* wi = static_cast<WaylandInterface*>(data);
    if (strcmp(interface, zwp_idle_inhibit_manager_v1_interface.name) == 0)
    {
        wi->m_inhibitManager = static_cast<zwp_idle_inhibit_manager_v1*>(
            wl_registry_bind(registry, name, &zwp_idle_inhibit_manager_v1_interface, 1));
    }
}

static const struct wl_registry_listener wl_registry_listener = { wl_global };

WaylandInterface::~WaylandInterface()
{
    if (m_inhibitor)
        zwp_idle_inhibitor_v1_destroy(m_inhibitor);
    if (m_inhibitManager)
        zwp_idle_inhibit_manager_v1_destroy(m_inhibitManager);
    if (m_surface)
        wl_surface_destroy(m_surface);
}

WaylandInterface::IdleInhibitResult WaylandInterface::inhibitIdle(bool inhibit)
{
    if (!m_surface)
        return IdleInhibitResult::SurfaceNotFound;
    if (!m_inhibitManager)
        return IdleInhibitResult::ManagerNotFound;

    if (inhibit)
    {
        m_inhibitor = zwp_idle_inhibit_manager_v1_create_inhibitor(m_inhibitManager, m_surface);
        return m_inhibitor ? IdleInhibitResult::Success : IdleInhibitResult::Failure;
    }
    else if (m_inhibitor)
    {
        zwp_idle_inhibitor_v1_destroy(m_inhibitor);
        m_inhibitor = nullptr;
    }

    return IdleInhibitResult::Success;
}

void WaylandInterface::initialize()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    auto* waylandApp = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
    if (!waylandApp)
        return;

    wl_compositor* compositor = waylandApp->compositor();
    wl_display* display = waylandApp->display();
#else
    QPlatformNativeInterface* nativeInterface = qGuiApp->platformNativeInterface();
    if (!nativeInterface)
        return;

    auto* compositor = static_cast<wl_compositor*>(nativeInterface->nativeResourceForIntegration("compositor"));
    auto* display = static_cast<wl_display*>(nativeInterface->nativeResourceForIntegration("display"));
#endif

    if (!compositor || !display || !(m_surface = wl_compositor_create_surface(compositor)))
        return;

    if (wl_registry* registry = wl_display_get_registry(display))
    {
        wl_registry_add_listener(registry, &wl_registry_listener, this);
        wl_display_roundtrip(display);
        wl_registry_destroy(registry);
    }
}
