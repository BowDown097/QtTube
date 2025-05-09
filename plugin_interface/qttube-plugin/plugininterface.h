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

    class PluginSettings
    {
    public:
        virtual ~PluginSettings() = default;

        const QString& configPath() const { return m_configPath; }
        virtual void init() = 0;
        virtual void save() = 0;
        virtual QWidget* window() { return nullptr; }

        template<typename T> requires std::derived_from<T, PluginSettings>
        static std::unique_ptr<T> create(const QString& key)
        {
            auto inst = std::make_unique<T>();
            inst->m_configPath =
                QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) +
                QDir::separator() +
                key.toLower().replace(' ', '-') +
                QDir::separator() +
                "settings.ini";
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
}

using QtTubePluginMetadataFunc = QtTube::PluginMetadata*(*)();
using QtTubePluginNewInstanceFunc = QtTube::PluginInterface*(*)();
using QtTubePluginSettingsFunc = QtTube::PluginSettings*(*)();
using QtTubePluginVersionFunc = const char*(*)();

#define DECLARE_QTTUBE_PLUGIN(PluginClass, SettingsClass, ...) \
    extern "C" \
    { \
        DLLEXPORT QtTube::PluginMetadata* metadata() { static QtTube::PluginMetadata md = { __VA_ARGS__ }; return &md; } \
        DLLEXPORT QtTube::PluginInterface* newInstance() { return new PluginClass; } \
        DLLEXPORT QtTube::PluginSettings* settings() \
        { \
            static std::unique_ptr<SettingsClass> s = QtTube::PluginSettings::create<SettingsClass>(metadata()->name); \
            return s.get(); \
        } \
        DLLEXPORT const char* targetVersion() { return QTTUBE_VERSION_NAME; } \
    }
