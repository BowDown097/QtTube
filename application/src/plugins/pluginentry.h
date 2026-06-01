#pragma once
#include "qttube-plugin/pluginfwd.h"
#include "qttube-plugin/pluginmetadata.h"
#include <QException>
#include <QFileInfo>
#include <QLibrary>

class PluginLoadException : public QException
{
public:
    explicit PluginLoadException(const QString& message) : m_message(message) {}
    PluginLoadException* clone() const override { return new PluginLoadException(*this); }
    const QString& message() const { return m_message; }
    void raise() const override { throw *this; }
private:
    QString m_message;
};

class PluginEntry
{
public:
    bool active{};
    QtTubePlugin::AuthStoreBase* authStore{};
    QFileInfo fileInfo;
    std::unique_ptr<QtTubePlugin::PluginInterface> interface;
    QtTubePlugin::PluginMetadata metadata;
    QtTubePluginPlayerFunc playerFunc{};
    QtTubePlugin::SettingsStore* settings{};

    virtual ~PluginEntry();
    PluginEntry(PluginEntry&&);
    PluginEntry(const PluginEntry&) = delete;
    PluginEntry& operator=(PluginEntry&&);
    PluginEntry& operator=(const PluginEntry&) = delete;

    virtual void initialize();
    virtual void unload() {}

    static std::unique_ptr<PluginEntry> create(QFileInfo&& fileInfo);
    static bool isPluginFile(const QString& fileName);
protected:
    explicit PluginEntry(QFileInfo&& fileInfo_);
    void checkMetadata();
    void checkTargetVersion(std::string_view targetVersion);
};

class NativePluginEntry : public PluginEntry
{
public:
    explicit NativePluginEntry(QFileInfo&& fileInfo_);
    void initialize() override;
    void unload() override;
private:
    QLibrary m_handle;
};
