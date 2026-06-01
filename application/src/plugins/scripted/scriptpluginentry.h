#pragma once
#include "plugins/pluginentry.h"
#include <quickjs++/quickjs_fwd.h>

struct ScriptPluginEntry : PluginEntry
{
    explicit ScriptPluginEntry(QFileInfo&& fileInfo_) : PluginEntry(std::move(fileInfo_)) {}
    void initialize() override;
};
