#pragma once
#include <QString>

#ifdef Q_OS_WIN
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

namespace QtTube
{
    struct PluginInterface
    {
        virtual void init() = 0;
        virtual ~PluginInterface() = default;
    };

    struct PluginMetadata
    {
        const char* name;
        const char* description;
        const char* image;
        const char* author;
        const char* url;
    };

    struct PluginSettings {};
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
        DLLEXPORT QtTube::PluginSettings* settings() { static SettingsClass s; return &s; } \
        DLLEXPORT const char* targetVersion() { return QTTUBE_VERSION_NAME; } \
    }
