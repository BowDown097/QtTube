#pragma once
#include "components/pluginauth.h"
#include "components/pluginsettings.h"
#include "components/replytypes.h"

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

        virtual BrowseReply* getHistory(
            const QString& query = {},
            std::any continuationData = {}) { return BrowseReply::create(); }
        virtual BrowseReply* getHome(std::any continuationData = {}) { return BrowseReply::create(); }
        virtual NotificationsReply* getNotifications(std::any continuationData = {}) { return NotificationsReply::create(); }
        virtual BrowseReply* getSearch(
            const QString& query = {},
            const QList<std::pair<QString, int>>& activeFilters = {}, // mapped as category -> index
            std::any continuationData = {}) { return BrowseReply::create(); }
        virtual BrowseReply* getSubFeed(std::any continuationData = {}) { return BrowseReply::create(); }
        virtual BrowseReply* getTrending(std::any continuationData = {}) { return BrowseReply::create(); }

        virtual PluginReply<void>* setNotificationPreference(std::any data = {}) { return PluginReply<void>::create(); }
        virtual PluginReply<void>* subscribe(std::any data = {}) { return PluginReply<void>::create(); }
        virtual PluginReply<void>* unsubscribe(std::any data = {}) { return PluginReply<void>::create(); }

        virtual void init() = 0;

        // mapped as category -> filters
        virtual const QList<std::pair<QString, QStringList>> searchFilters() const { return {}; }
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

#define GET_MACRO(_1, _2, _3, _4, NAME, ...) NAME
#define DECLARE_QTTUBE_PLUGIN(...) \
    GET_MACRO(__VA_ARGS__, DECLARE_QTTUBE_PLUGIN4, DECLARE_QTTUBE_PLUGIN3, DECLARE_QTTUBE_PLUGIN2, DECLARE_QTTUBE_PLUGIN1)(__VA_ARGS__)

#define DECLARE_QTTUBE_PLUGIN2(PluginClass, MetadataInstance) \
    extern "C" \
    { \
        DLLEXPORT const char* targetVersion() { return QTTUBE_VERSION_NAME; } \
        DLLEXPORT QtTube::PluginInterface* newInstance() { return new PluginClass; } \
        DLLEXPORT QtTube::PluginMetadata* metadata() { return &MetadataInstance; } \
    }

#define DECLARE_QTTUBE_PLUGIN3(PluginClass, MetadataInstance, SettingsClass) \
    extern "C" \
    { \
        DLLEXPORT const char* targetVersion() { return QTTUBE_VERSION_NAME; } \
        DLLEXPORT QtTube::PluginInterface* newInstance() { return new PluginClass; } \
        DLLEXPORT QtTube::PluginMetadata* metadata() { return &MetadataInstance; } \
        DLLEXPORT QtTube::PluginSettings* settings() \
        { \
            static std::unique_ptr<SettingsClass> s = QtTube::PluginSettings::create<SettingsClass>(metadata()->name); \
            return s.get(); \
        } \
    }

#define DECLARE_QTTUBE_PLUGIN4(PluginClass, MetadataInstance, SettingsClass, AuthClass) \
    extern "C" \
    { \
        DLLEXPORT const char* targetVersion() { return QTTUBE_VERSION_NAME; } \
        DLLEXPORT QtTube::PluginInterface* newInstance() { return new PluginClass; } \
        DLLEXPORT QtTube::PluginMetadata* metadata() { return &MetadataInstance; } \
        DLLEXPORT QtTube::PluginSettings* settings() \
        { \
            static std::unique_ptr<SettingsClass> s = QtTube::PluginSettings::create<SettingsClass>(metadata()->name); \
            return s.get(); \
        } \
        DLLEXPORT QtTube::PluginAuth* auth() \
        { \
            static std::unique_ptr<AuthClass> a = QtTube::PluginAuth::create<AuthClass>(metadata()->name); \
            return a.get(); \
        } \
    }
