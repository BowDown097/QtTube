#pragma once
#include "qttube-plugin/plugininterface.h"
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
    bool active{};
    QtTubePlugin::AuthStoreBase* auth{};
    QFileInfo fileInfo;
    std::unique_ptr<QLibrary, QLibraryDeleter> handle;
    std::unique_ptr<QtTubePlugin::PluginInterface> interface;
    QtTubePlugin::PluginMetadata metadata;
    QtTubePluginPlayerFunc playerFunc{};
    QtTubePlugin::SettingsStore* settings{};
};

class PluginManager
{
public:
    PluginData* activePlugin();
    bool containsPlugin(const QString& name);
    PluginData* findPlugin(const QString& name);
    PluginData* loadAndInitPlugin(PluginData&& plugin);
    PluginData* loadAndInitPlugin(const QFileInfo& fileInfo);
    QList<PluginData*> loadedPlugins();
    PluginData openPlugin(const QFileInfo& fileInfo);
    void reloadPlugins();

    static const QStringList& libraryLoadDirs();
    static const QStringList& pluginLoadDirs();
private:
    std::unordered_map<QString, PluginData, CaseInsensitiveHash, CaseInsensitiveEqual> m_loadedPlugins;

    void checkPluginMetadata(const PluginData& plugin);
    void checkPluginTargetVersion(const PluginData& plugin);
};
