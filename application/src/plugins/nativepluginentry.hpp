#pragma once
#include "pluginentry.hpp"
#include <QLibrary>

class NativePluginEntry : public PluginEntry
{
public:
    explicit NativePluginEntry(const QFileInfo& fileInfo);
    ~NativePluginEntry() override { m_handle.unload(); }
    void initialize() override;
private:
    QLibrary m_handle;
};