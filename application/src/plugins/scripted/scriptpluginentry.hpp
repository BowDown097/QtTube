#pragma once
#include "plugins/pluginentry.hpp"
#include <quickjs++/quickjs_fwd.h>

class ScriptPluginEntry : public PluginEntry
{
public:
    explicit ScriptPluginEntry(const QFileInfo& fileInfo) : PluginEntry(fileInfo) {}
    void initialize() override;
private:
    static QtTubePlugin::PluginMetadata createMetadata(const qjs::value& metadata);
    static void setupContext(qjs::context& context, const QDir& dir);
    static void setupLocalCache(qjs::module& mod);
    static void setupWebEngine(qjs::context& ctx);
};
