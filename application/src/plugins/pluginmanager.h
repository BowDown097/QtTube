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

class PluginData
{
public:
    bool active{};
    QtTube::PluginAuth* auth{};
    QFileInfo fileInfo;
    std::unique_ptr<QLibrary, QLibraryDeleter> handle;
    std::unique_ptr<QtTube::PluginInterface> interface;
    QtTube::PluginMetadata* metadata{};
    QtTube::PluginSettings* settings{};

    QtTube::PluginPlayer* createPlayer(QWidget* parent)
    {
        if (!m_playerFunc)
            m_playerFunc = QtTubePluginPlayerFunc(handle->resolve("player"));

        if (m_playerFunc)
            return m_playerFunc(parent);
        else
            return nullptr;
    }
private:
    QtTubePluginPlayerFunc m_playerFunc{};
};

class PluginManager
{
public:
    PluginData* activePlugin();
    PluginData* findPlugin(const QString& name);
    const QList<PluginData*> loadedPlugins();
    void reloadPlugins();
private:
    std::unordered_map<QString, PluginData, CaseInsensitiveHash, CaseInsensitiveEqual> m_loadedPlugins;

    bool checkPluginTargetVersion(const QFileInfo& fileInfo);
    std::optional<PluginData> loadPlugin(const QFileInfo& fileInfo);
};
