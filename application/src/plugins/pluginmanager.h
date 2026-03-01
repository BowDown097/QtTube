#pragma once
#include "pluginbrowser.h"
#include "pluginentry.h"

struct CaseInsensitiveEqual
{
    bool operator()(const QString& lhs, const QString& rhs) const
    {
        return lhs.compare(rhs, Qt::CaseInsensitive) == 0;
    }
};

struct CaseInsensitiveHash
{
    [[nodiscard]] std::size_t operator()(const QString& str) const
    {
        return std::hash<QString>()(str.toCaseFolded());
    }
};

class PluginManager : public QObject
{
    Q_OBJECT
public:
    PluginEntry* activePlugin();
    bool containsPlugin(const QString& name);
    PluginEntry* findPlugin(const QString& name);
    bool hasAuthenticated() const;
    bool hasLoadablePlugins() const { return m_foundPluginFile; }
    QList<PluginEntry*> loadedPlugins();
    PluginEntry* registerPlugin(QFileInfo&& fileInfo);
    void reloadPlugins();
    std::unordered_map<QString, ReleaseData>& updatablePlugins() { return m_updatablePlugins; }

    static const QList<QDir>& libraryLoadDirs();
    static const QList<QDir>& pluginLoadDirs();
private:
    bool m_foundPluginFile{};
    std::unordered_map<QString, PluginEntry, CaseInsensitiveHash, CaseInsensitiveEqual> m_loadedPlugins;
    std::unordered_map<QString, ReleaseData> m_updatablePlugins;

    void checkUpdate(const QString& pluginName, const QFileInfo& updateFile);
    QList<QFileInfo> getPluginsToLoad(QString& activePluginName);
signals:
    void foundUpdate(const QString& name, const ReleaseData& data);
};
