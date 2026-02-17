#pragma once
#include "qttube-plugin/plugininterface.h"
#include "pluginbrowser.h"
#include <QFileInfo>
#include <QLibrary>

struct CaseInsensitiveEqual
{
    bool operator()(const QString& lhs, const QString& rhs) const
    {
        return lhs.compare(rhs, Qt::CaseInsensitive) == 0;
    }
};

struct CaseInsensitiveHash
{
    std::size_t operator()(const QString& str) const
    {
        return std::hash<QString>()(str.toLower());
    }
};

struct QLibraryDeleter
{
    void operator()(QLibrary* p) const
    {
        p->unload();
        p->deleteLater();
    }
};

using QLibraryPtr = std::unique_ptr<QLibrary, QLibraryDeleter>;

class PluginLoadException : public QException
{
public:
    void raise() const override { throw *this; }
    PluginLoadException* clone() const override { return new PluginLoadException(*this); }

    explicit PluginLoadException(const QString& message) : m_message(message) {}
    const QString& message() const { return m_message; }
private:
    QString m_message;
};

struct PluginData
{
    QFileInfo fileInfo;
    QtTubePlugin::PluginMetadata metadata;

    std::unique_ptr<QLibrary, QLibraryDeleter> handle;
    std::unique_ptr<QtTubePlugin::PluginInterface> interface;

    QtTubePlugin::AuthStoreBase* auth{};
    QtTubePluginPlayerFunc playerFunc{};
    QtTubePlugin::SettingsStore* settings{};

    bool active{};
};

class PluginManager : public QObject
{
    Q_OBJECT
public:
    PluginData* activePlugin();
    bool containsPlugin(const QString& name);
    PluginData* findPlugin(const QString& name);
    bool hasAuthenticated() const;
    bool hasLoadablePlugins() const { return m_foundPluginFile; }
    PluginData* loadAndInitPlugin(PluginData&& plugin);
    PluginData* loadAndInitPlugin(const QFileInfo& fileInfo);
    QList<PluginData*> loadedPlugins();
    PluginData openPlugin(const QFileInfo& fileInfo);
    void reloadPlugins();
    std::unordered_map<QString, ReleaseData>& updatablePlugins() { return m_updatablePlugins; }

    static const QList<QDir>& libraryLoadDirs();
    static const QList<QDir>& pluginLoadDirs();
private:
    std::unordered_map<QString, PluginData, CaseInsensitiveHash, CaseInsensitiveEqual> m_loadedPlugins;
    std::unordered_map<QString, ReleaseData> m_updatablePlugins;
    bool m_foundPluginFile{};

    void checkPluginMetadata(const PluginData& plugin);
    void checkPluginTargetVersion(const PluginData& plugin);
    void checkUpdate(const QString& pluginName, const QFileInfo& updateFile);
signals:
    void foundUpdate(const QString& name, const ReleaseData& data);
};
