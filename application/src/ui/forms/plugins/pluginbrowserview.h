#pragma once
#include "basepluginentry.h"
#include "plugins/pluginbrowser.h"

namespace Ui {
class PluginBrowserView;
}

class QHBoxLayout;
class QPushButton;
class QVBoxLayout;

class PluginBrowserView : public QWidget
{
    Q_OBJECT
public:
    explicit PluginBrowserView(QWidget* parent = nullptr);
    ~PluginBrowserView();
    void startPopulating();
protected:
    void keyPressEvent(QKeyEvent* event) override;
private:
    PluginBrowser* m_browser;
    Ui::PluginBrowserView* ui;
private slots:
    void error(const QString& context, const QString& message);
    void downloadBuild(const ReleaseData& data);
    void gotPluginMetadata(BasePluginEntry* entry, const PluginEntryMetadataPtr& metadata);
    void gotReleaseData(const PluginEntryMetadataPtr& metadata, const std::optional<ReleaseData>& data);
    void gotRepositories(const QList<RepositoryItemPtr>& items);
};

class PluginBrowserViewEntry : public BasePluginEntry
{
    Q_OBJECT
public:
    explicit PluginBrowserViewEntry(QWidget* parent = nullptr);
    void setData(const PluginEntryMetadata& metadata) override;
private:
    QPushButton* m_installButton;
signals:
    void installButtonClicked();
};
