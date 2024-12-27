#include "waylandinterface.h"
#include "wayland/ext-idle-inhibit-unstable-v1.h"
#include <QGuiApplication>

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <QtGui/qpa/qplatformwindow_p.h>
#else
#include <QtGui/qpa/qplatformnativeinterface.h>
#endif

static void wl_global(void* data, wl_registry* registry, std::uint32_t name, const char* interface, std::uint32_t version)
{
    WaylandInterface* wi = static_cast<WaylandInterface*>(data);
    if (!strcmp(interface, zwp_idle_inhibit_manager_v1_interface.name))
    {
        wi->setInhibitManager(static_cast<zwp_idle_inhibit_manager_v1*>(
            wl_registry_bind(registry, name, &zwp_idle_inhibit_manager_v1_interface, 1)));
    }
}

static void wl_global_remove(void* data, wl_registry* registry, std::uint32_t name) {}

static const struct wl_registry_listener wl_registry_listener = { wl_global, wl_global_remove };

WaylandInterface::~WaylandInterface()
{
    if (m_inhibitor)
        zwp_idle_inhibitor_v1_destroy(m_inhibitor);
    if (m_inhibitManager)
        zwp_idle_inhibit_manager_v1_destroy(m_inhibitManager);
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
    else
    {
        zwp_idle_inhibitor_v1_destroy(m_inhibitor);
        m_inhibitor = nullptr;
        return INHIBIT_SUCCESS;
    }
}

void WaylandInterface::initialize(QWindow* windowHandle)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    auto waylandApp = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
    auto waylandWindow = windowHandle->nativeInterface<QNativeInterface::Private::QWaylandWindow>();
    if (!waylandApp || !waylandWindow)
        return;

    wl_display* display = waylandApp->display();
    m_surface = waylandWindow->surface();
#else
    QPlatformNativeInterface* nativeInterface = qGuiApp->platformNativeInterface();
    if (!nativeInterface)
        return;

    wl_display* display = static_cast<wl_display*>(nativeInterface->nativeResourceForWindow("display", nullptr));
    m_surface = static_cast<wl_surface*>(nativeInterface->nativeResourceForWindow("surface", windowHandle));
#endif

    if (!display || !m_surface)
        return;

    if (wl_registry* registry = wl_display_get_registry(display))
    {
        wl_registry_add_listener(registry, &wl_registry_listener, this);
        wl_display_roundtrip(display);
    }
}
