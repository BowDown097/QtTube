#pragma once
#include "qttube-plugin/plugininterface.h"
#include <memory>
#include <QLibrary>

struct QLibraryDeleter
{
	void operator()(QLibrary* p) const
	{
    	p->unload();
    	p->deleteLater();
	}
};

class PluginLoadException : public QException
{
public:
    void raise() const override { throw *this; }
    PluginLoadException* clone() const override { return new PluginLoadException(*this); }

    explicit PluginLoadException(const QString& message) : m_message(message) {}
    const QString& message() const { return m_message; }
private:
    QString m_message;
};

class PluginEntry
{
    friend class PluginManager;
public:
    bool active{};
    std::unique_ptr<QLibrary, QLibraryDeleter> handle; 
    QtTubePlugin::AuthStoreBase* authStore{};
    QFileInfo fileInfo;
    std::unique_ptr<QtTubePlugin::PluginInterface> interface;
    QtTubePlugin::PluginMetadata metadata;
    QtTubePluginPlayerFunc playerFunc{};
    QtTubePlugin::SettingsStore* settings{};

    static bool isPluginFile(const QFileInfo& info);
    static bool isPluginFile(const QString& fileName);
private:
    explicit PluginEntry(QFileInfo&& info);
    void initialize();

    void checkMetadata();
    void checkTargetVersion(std::string_view targetVersion);
    void loadAsNative();
    void loadAsScript();
};
