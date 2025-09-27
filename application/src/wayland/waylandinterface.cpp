#include "waylandinterface.h"
#include "wayland/ext-idle-inhibit-unstable-v1.h"
#include <QGuiApplication>

#if QT_VERSION < QT_VERSION_CHECK(6, 5, 0)
#include <QtGui/qpa/qplatformnativeinterface.h>
#endif

static void wl_global(void* data, wl_registry* registry, std::uint32_t name, const char* interface, std::uint32_t version)
{
    WaylandInterface* wi = static_cast<WaylandInterface*>(data);
    if (strcmp(interface, zwp_idle_inhibit_manager_v1_interface.name) == 0)
    {
        wi->setInhibitManager(static_cast<zwp_idle_inhibit_manager_v1*>(
            wl_registry_bind(registry, name, &zwp_idle_inhibit_manager_v1_interface, 1)));
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

WaylandInterface::InhibitIdleResult WaylandInterface::inhibitIdle(bool inhibit)
{
    if (!m_surface)
        return INHIBIT_SURFACE_NOT_FOUND;
    if (!m_inhibitManager)
        return INHIBIT_MANAGER_NOT_FOUND;

    if (inhibit)
    {
        m_inhibitor = zwp_idle_inhibit_manager_v1_create_inhibitor(m_inhibitManager, m_surface);
        return m_inhibitor ? INHIBIT_SUCCESS : INHIBIT_FAILURE;
    }
    else if (m_inhibitor)
    {
        zwp_idle_inhibitor_v1_destroy(m_inhibitor);
        m_inhibitor = nullptr;
    }

    return INHIBIT_SUCCESS;
}

void WaylandInterface::initialize()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    QNativeInterface::QWaylandApplication* waylandApp = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
    if (!waylandApp)
        return;

    wl_compositor* compositor = waylandApp->compositor();
    wl_display* display = waylandApp->display();
#else
    QPlatformNativeInterface* nativeInterface = qGuiApp->platformNativeInterface();
    if (!nativeInterface)
        return;

    wl_compositor* compositor = static_cast<wl_compositor*>(nativeInterface->nativeResourceForIntegration("compositor"));
    wl_display* display = static_cast<wl_display*>(nativeInterface->nativeResourceForIntegration("display"));
#endif

    if (!compositor || !display || !(m_surface = wl_compositor_create_surface(compositor)))
        return;

    if (wl_registry* registry = wl_display_get_registry(display))
    {
        wl_registry_add_listener(registry, &wl_registry_listener, this);
        wl_display_roundtrip(display);
    }
}
