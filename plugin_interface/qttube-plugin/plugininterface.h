#pragma once
#include "components/pluginauth.h"
#include "components/pluginsettings.h"

#ifdef Q_OS_WIN
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

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
}

using QtTubePluginAuthFunc = QtTube::PluginAuth*(*)();
using QtTubePluginMetadataFunc = QtTube::PluginMetadata*(*)();
using QtTubePluginNewInstanceFunc = QtTube::PluginInterface*(*)();
using QtTubePluginSettingsFunc = QtTube::PluginSettings*(*)();
using QtTubePluginVersionFunc = const char*(*)();

#define DECLARE_QTTUBE_PLUGIN(PluginClass, SettingsClass, AuthClass, ...) \
    extern "C" \
    { \
        DLLEXPORT QtTube::PluginMetadata* metadata() { static QtTube::PluginMetadata md = { __VA_ARGS__ }; return &md; } \
        DLLEXPORT QtTube::PluginInterface* newInstance() { return new PluginClass; } \
        DLLEXPORT QtTube::PluginAuth* auth() \
        { \
            static std::unique_ptr<AuthClass> a = QtTube::PluginAuth::create<AuthClass>(metadata()->name); \
            return a.get(); \
        } \
        DLLEXPORT QtTube::PluginSettings* settings() \
        { \
            static std::unique_ptr<SettingsClass> s = QtTube::PluginSettings::create<SettingsClass>(metadata()->name); \
            return s.get(); \
        } \
        DLLEXPORT const char* targetVersion() { return QTTUBE_VERSION_NAME; } \
    }
