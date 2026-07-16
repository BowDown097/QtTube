#pragma once
#include "pluginentry.hpp"
#include <QLibrary>

class NativePluginEntry : public PluginEntry
{
public:
    explicit NativePluginEntry(QFileInfo&& fileInfo_);
    void initialize() override;
    void unload() override;
private:
    QLibrary m_handle;
};