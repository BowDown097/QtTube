#pragma once
#include "qttube-plugin/plugininterface.h"
#include <QFileInfo>
#include <QLibrary>

struct QLibraryDeleter
{
    void operator()(QLibrary* p) const
    {
        p->unload();
        p->deleteLater();
    }
};

struct PluginData
{
    QtTube::PluginAuth* auth{};
    QFileInfo fileInfo;
    std::unique_ptr<QLibrary, QLibraryDeleter> handle;
    std::unique_ptr<QtTube::PluginInterface> interface;
    QtTube::PluginMetadata* metadata{};
    QtTube::PluginSettings* settings{};
};

class PluginManager
{
public:
    const PluginData* findPlugin(const QString& name) const;
    QList<const PluginData*> plugins() const;
    void reloadPlugins();
private:
    std::unordered_map<QString, PluginData> m_plugins;

    bool checkPluginTargetVersion(const QFileInfo& fileInfo);
    std::optional<PluginData> loadPlugin(const QFileInfo& fileInfo);
};
