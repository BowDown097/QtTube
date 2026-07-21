#pragma once
#include <qttube-plugin/plugininterface.h>
#include <quickjs++/context.h>

struct ScriptPluginInterface : QtTubePlugin::PluginInterface
{
    std::unique_ptr<qjs::context> context;
    qjs::value moduleNamespace;
    QString pluginName;

    explicit ScriptPluginInterface(
        const QString& pluginName,
        qjs::value&& moduleNamespace,
        std::unique_ptr<qjs::context>&& context)
        : context(std::move(context)),
          moduleNamespace(std::move(moduleNamespace)),
          pluginName(pluginName) {}

    void init() override;
    void registerProviders(QtTubePlugin::ProviderRegistry& reg) override;
};
