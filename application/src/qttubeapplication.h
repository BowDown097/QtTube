#pragma once
#include "plugins/pluginmanager.h"
#include "stores/settingsstore.h"
#include <QApplication>

#ifdef QTTUBE_HAS_WAYLAND
#include "wayland/waylandinterface.h"
#endif

#define qtTubeApp (static_cast<QtTubeApplication*>(QCoreApplication::instance()))

class QtTubeApplication final : public QApplication
{
    Q_OBJECT
public:
    QtTubeApplication(int& argc, char** argv) : QApplication(argc, argv) {}
    bool notify(QObject* receiver, QEvent* event) override;

    void doInitialSetup();
    void handleUrlOrID(const QString& in);

    PluginManager& plugins() { return m_plugins; }
    SettingsStore& settings() { return m_settings; }

#ifdef QTTUBE_HAS_WAYLAND
    WaylandInterface& waylandInterface() { return m_waylandInterface; }
#endif
private:
    PluginManager m_plugins;
    SettingsStore m_settings;

#ifdef QTTUBE_HAS_WAYLAND
    WaylandInterface m_waylandInterface;
#endif
signals:
    void activePluginChanged(PluginData* active);
};
