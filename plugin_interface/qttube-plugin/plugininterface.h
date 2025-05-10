#pragma once
#include <QDir>
#include <QSettings>
#include <QStandardPaths>

#ifdef Q_OS_WIN
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

class QWidget;

namespace QtTube
{
    struct PluginInterface
    {
        virtual ~PluginInterface() = default;
        virtual void init() = 0;
    };

    struct PluginMetadata
    {
        const char* name = "";
        const char* description = "";
        const char* image = "";
        const char* author = "";
        const char* url = "";
    };

    class ConfigStore
    {
    public:
        virtual ~ConfigStore() = default;
        virtual void init() = 0;
        virtual void save() = 0;

        const QString& configPath() const { return m_configPath; }

        template<typename T> requires std::derived_from<T, ConfigStore>
        static std::unique_ptr<T> create(const QString& key, const QString& filename)
        {
            auto inst = std::make_unique<T>();
            inst->m_configPath =
                QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) +
                QDir::separator() +
                key.toLower().replace(' ', '-') +
                QDir::separator() +
                filename;
            return inst;
        }
    protected:
        template<typename T> requires std::constructible_from<QVariant, T>
        void readIntoList(QSettings& settings, QList<T>& list, const QString& prefix, const QString& key)
        {
            list.clear();
            for (int i = 0; i < settings.beginReadArray(prefix); ++i)
            {
                settings.setArrayIndex(i);
                list.append(settings.value(key).value<T>());
            }
            settings.endArray();
        }

        template<typename T> requires std::constructible_from<QVariant, T>
        void writeList(QSettings& settings, const QList<T>& list, const QString& prefix, const QString& key)
        {
            settings.beginWriteArray(prefix);
            for (int i = 0; i < list.size(); ++i)
            {
                settings.setArrayIndex(i);
                settings.setValue(key, list.at(i));
            }
            settings.endArray();
        }
    private:
        QString m_configPath;
    };

    struct PluginAuthentication : ConfigStore
    {
        virtual ~PluginAuthentication() = default;
        virtual void clear() { QSettings(configPath(), QSettings::IniFormat).clear(); }

        template<typename T> requires std::derived_from<T, PluginAuthentication>
        static std::unique_ptr<T> create(const QString& key)
        {
            return ConfigStore::create<T>(key, "auth.ini");
        }
    };

    struct PluginSettings : ConfigStore
    {
        virtual ~PluginSettings() = default;
        virtual QWidget* window() { return nullptr; }

        template<typename T> requires std::derived_from<T, PluginSettings>
        static std::unique_ptr<T> create(const QString& key)
        {
            return ConfigStore::create<T>(key, "settings.ini");
        }
    };
}

using QtTubePluginMetadataFunc = QtTube::PluginMetadata*(*)();
using QtTubePluginNewInstanceFunc = QtTube::PluginInterface*(*)();
using QtTubePluginSettingsFunc = QtTube::PluginSettings*(*)();
using QtTubePluginVersionFunc = const char*(*)();

#define DECLARE_QTTUBE_PLUGIN(PluginClass, SettingsClass, AuthClass, ...) \
    extern "C" \
    { \
        DLLEXPORT QtTube::PluginMetadata* metadata() { static QtTube::PluginMetadata md = { __VA_ARGS__ }; return &md; } \
        DLLEXPORT QtTube::PluginInterface* newInstance() { return new PluginClass; } \
        DLLEXPORT QtTube::PluginAuthentication* authentication() \
        { \
            static std::unique_ptr<AuthClass> a = QtTube::PluginAuthentication::create<AuthClass>(metadata()->name); \
            return a.get(); \
        } \
        DLLEXPORT QtTube::PluginSettings* settings() \
        { \
            static std::unique_ptr<SettingsClass> s = QtTube::PluginSettings::create<SettingsClass>(metadata()->name); \
            return s.get(); \
        } \
        DLLEXPORT const char* targetVersion() { return QTTUBE_VERSION_NAME; } \
    }
