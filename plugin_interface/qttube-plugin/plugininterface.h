#pragma once
#include <QString>

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
}

using QtTubePluginLoadFunc = QtTube::PluginInterface*(*)();
using QtTubePluginVersionFunc = const char*(*)();
constexpr const char* TARGET_VERSION = QTTUBE_VERSION_NAME;

#define DECLARE_QTTUBE_PLUGIN(PluginClass, ...) \
    extern "C" \
    { \
        QtTube::PluginMetadata metadata() { return { __VA_ARGS__ }; } \
        QtTube::PluginInterface* newInstance() { return new PluginClass; } \
        const char* targetVersion() { return TARGET_VERSION; } \
    }
