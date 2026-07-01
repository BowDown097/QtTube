#pragma once
#include "pluginentry.hpp"
#include "releasedata.hpp"
#include "utils/hashfunctions.hpp"

class PluginManager : public QObject
{
    Q_OBJECT
public:
    PluginEntry* activePlugin();
    bool containsPlugin(const QString& name);
    PluginEntry* findPlugin(const QString& name);
    bool hasAuthenticated() const;
    bool hasLoadablePlugins() const { return m_foundPluginFile; }
    const QList<PluginEntry*> loadedPlugins();
    PluginEntry* registerPlugin(QFileInfo&& fileInfo);
    void reloadPlugins();
    std::unordered_map<QString, ReleaseData>& updatablePlugins() { return m_updatablePlugins; }

    static const QList<QDir>& libraryLoadDirs();
    static const QList<QDir>& pluginLoadDirs();
private:
    bool m_foundPluginFile{};
    std::unordered_map<
        QString, std::unique_ptr<PluginEntry>,
        CaseInsensitiveHash, CaseInsensitiveEqual> m_loadedPlugins;
    std::unordered_map<QString, ReleaseData> m_updatablePlugins;

    void checkUpdate(const QString& pluginName, const QFileInfo& updateFile);
    QList<QFileInfo> getPluginsToLoad(QString& activePluginName);
signals:
    void foundUpdate(const QString& name, const ReleaseData& data);
};
