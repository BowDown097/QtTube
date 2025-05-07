#pragma once
#include "stores/credentialsstore.h"
#include "stores/settingsstore.h"
#include <QApplication>

#ifdef QTTUBE_HAS_WAYLAND
#include "wayland/waylandinterface.h"
#endif

#define qtTubeApp (static_cast<QtTubeApplication*>(QCoreApplication::instance()))

class QtTubeApplication final : public QApplication
{
public:
    QtTubeApplication(int& argc, char** argv) : QApplication(argc, argv) {}
    bool notify(QObject* receiver, QEvent* event) override;

    void doInitialSetup();

    CredentialsStore& creds() { return m_creds; }
    SettingsStore& settings() { return m_settings; }

#ifdef QTTUBE_HAS_WAYLAND
    WaylandInterface& waylandInterface() { return m_waylandInterface; }
#endif
private:
    CredentialsStore m_creds;
    SettingsStore m_settings;

#ifdef QTTUBE_HAS_WAYLAND
    WaylandInterface m_waylandInterface;
#endif
};
