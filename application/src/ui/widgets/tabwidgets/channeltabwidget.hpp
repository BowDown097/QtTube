#pragma once
#include "browsertabwidget.hpp"
#include "plugins/pluginentry.hpp"

class ChannelTabWidget final : public BrowserTabWidget
{
public:
    explicit ChannelTabWidget(QWidget* parent = nullptr);
    void browse() override;
    void clear();
    void reloadCurrentTab(ContinuableListWidget* list) override;
    void setData(
        PluginEntry* plugin,
        const QString& channelId,
        const QList<QtTubePlugin::ChannelTabData>& tabs);
private:
    QString m_channelId;
    PluginEntry* m_plugin;
    QList<std::any> m_tabDatas;
};
