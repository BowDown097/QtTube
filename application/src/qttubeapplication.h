#pragma once
#include "plugins/pluginmanager.h"
#include "stores/settingsstore.h"
#include <QApplication>
#include <QCommandLineParser>

#ifdef QTTUBE_HAS_WAYLAND
#include "wayland/waylandinterface.h"
#endif

#define qtTubeApp (static_cast<QtTubeApplication*>(QCoreApplication::instance()))

class QtTubeApplication final : public QApplication
{
    Q_OBJECT
public:
    QtTubeApplication(int& argc, char** argv);
    bool notify(QObject* receiver, QEvent* event) override;

    static bool isPortableBuild();
    static bool isSelfContainedBuild();

    void doInitialSetup();
    void handleUrlOrID(const QString& in);

    QCommandLineParser& commandLineParser() { return m_commandLineParser; }
    PluginManager& plugins() { return m_plugins; }
    SettingsStore& settings() { return m_settings; }

#ifdef QTTUBE_HAS_WAYLAND
    WaylandInterface& waylandInterface() { return m_waylandInterface; }
#endif
private:
    QCommandLineParser m_commandLineParser;
    PluginManager m_plugins;
    SettingsStore m_settings;

#ifdef QTTUBE_HAS_WAYLAND
    WaylandInterface m_waylandInterface;
#endif
signals:
    void activePluginChanged(PluginData* active);
};
