#pragma once
#include "pluginentry.hpp"
#include <QLibrary>

class NativePluginEntry : public PluginEntry
{
public:
    explicit NativePluginEntry(QFileInfo&& fileInfo_);
    ~NativePluginEntry() override { m_handle.unload(); }
    void initialize() override;
private:
    QLibrary m_handle;
};