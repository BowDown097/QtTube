#include "channeltabwidget.hpp"
#include "ui/browsehelper.hpp"

ChannelTabWidget::ChannelTabWidget(QWidget* parent)
    : BrowserTabWidget(parent) {}

void ChannelTabWidget::browse()
{
    for (int i = 0; i < count(); ++i)
        if (auto* list = qobject_cast<ContinuableListWidget*>(widget(i)))
            list->clear();

    int index = currentIndex();
    auto* list = qobject_cast<ContinuableListWidget*>(currentWidget());
    BrowseHelper::instance()->browseChannel(m_plugin, list, index, m_channelId, m_tabDatas[index]);
}

void ChannelTabWidget::clear()
{
    BrowserTabWidget::clear();
    m_tabDatas.clear();
}

void ChannelTabWidget::reloadCurrentTab(ContinuableListWidget* list)
{
    list->clear();
    int index = currentIndex();
    BrowseHelper::instance()->browseChannel(m_plugin, list, index, m_channelId, m_tabDatas[index]);
}

void ChannelTabWidget::setData(
    PluginEntry* plugin,
    const QString& channelId,
    const QList<QtTubePlugin::ChannelTabData>& tabs)
{
    if (tabs.isEmpty())
        return;

    m_tabDatas.clear();

    m_channelId = channelId;
    m_plugin = plugin;

    ContinuableListWidget* firstListWidget{};

    for (qsizetype i = 0; i < tabs.size(); ++i)
    {
        m_tabDatas.append(tabs[i].requestData);

        ContinuableListWidget* list = new ContinuableListWidget;
        if (!firstListWidget)
            firstListWidget = list;
        addTab(list, tabs[i].title);
        CONTINUE_CONNECT(list, browseChannel, i, m_channelId, m_tabDatas[i]);
    }

    BrowseHelper::instance()->processChannelTabItems(firstListWidget, m_plugin, tabs.front().items);
}