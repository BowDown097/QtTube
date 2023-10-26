#ifndef QTTUBEAPPLICATION_H
#define QTTUBEAPPLICATION_H
#include "stores/credentialsstore.h"
#include "stores/settingsstore.h"
#include <QApplication>

#define qtTubeApp (static_cast<QtTubeApplication*>(QCoreApplication::instance()))

class QtTubeApplication final : public QApplication
{
public:
    QtTubeApplication(int& argc, char** argv) : QApplication(argc, argv) {}
    bool notify(QObject* receiver, QEvent* event) override;

    CredentialsStore& creds() { return m_creds; }
    SettingsStore& settings() { return m_settings; }
private:
    CredentialsStore m_creds;
    SettingsStore m_settings;
};

#endif // QTTUBEAPPLICATION_H
