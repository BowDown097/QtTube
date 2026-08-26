#pragma once
#include "plugins/pluginentry.hpp"
#include <quickjs++/quickjs_fwd.h>

struct ScriptPluginEntry : PluginEntry
{
    explicit ScriptPluginEntry(const QFileInfo& fileInfo) : PluginEntry(fileInfo) {}
    void initialize() override;
};
