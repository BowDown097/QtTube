#include "pluginbrowserview.h"
#include "ui_pluginbrowserview.h"
#include "pluginbuilddownloader.h"
#include "qttubeapplication.h"
#include "utils/uiutils.h"
#include <QMessageBox>
#include <QPushButton>

PluginBrowserView::PluginBrowserView(QWidget* parent)
    : QWidget(parent), m_browser(new PluginBrowser(this)), ui(new Ui::PluginBrowserView)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    connect(m_browser, &PluginBrowser::error, this, &PluginBrowserView::error);
    connect(m_browser, &PluginBrowser::gotNightlyBuild, this, &PluginBrowserView::downloadBuild);
    connect(m_browser, &PluginBrowser::gotPluginMetadata, this, &PluginBrowserView::gotPluginMetadata);
    connect(m_browser, &PluginBrowser::gotReleaseData, this, &PluginBrowserView::gotReleaseData);
    connect(m_browser, &PluginBrowser::gotRepositories, this, &PluginBrowserView::gotRepositories);
}

PluginBrowserView::~PluginBrowserView()
{
    delete ui;
}

void PluginBrowserView::error(const QString& context, const QString& message)
{
    QMessageBox::critical(this, "Failed " + context, message);
}

void PluginBrowserView::downloadBuild(const ReleaseData& data)
{
    // should get deleted on finish/close
    PluginBuildDownloader* downloader = new PluginBuildDownloader(data);
    downloader->show();
}

void PluginBrowserView::gotPluginMetadata(BasePluginEntry* entry, const PluginEntryMetadataPtr& metadata)
{
    if (!metadata)
    {
        entry->deleteLater();
        return;
    }

    PluginBrowserViewEntry* viewEntry = static_cast<PluginBrowserViewEntry*>(entry);
    viewEntry->setData(*metadata);
    viewEntry->show();
    connect(viewEntry, &PluginBrowserViewEntry::installButtonClicked, this, [this, metadata] {
        m_browser->tryGetReleaseData(metadata);
    });
}

void PluginBrowserView::gotReleaseData(
    const PluginEntryMetadataPtr& metadata, const std::optional<ReleaseData>& data)
{
    if (!data)
    {
        QMessageBox::StandardButton button = QMessageBox::question(
            this, "No Release Found",
            "No release has been found for this plugin. Would you like to try to get a nightly build?");
        if (button == QMessageBox::Yes)
            m_browser->tryGetNightlyBuild(metadata);
        return;
    }

    downloadBuild(data.value());
}

void PluginBrowserView::gotRepositories(const QList<RepositoryItemPtr>& items)
{
    for (const RepositoryItemPtr& item : items)
    {
        PluginBrowserViewEntry* entry = new PluginBrowserViewEntry;
        entry->hide();
        UIUtils::addWidgetToList(ui->listWidget, entry);

        if (PluginEntryMetadataPtr metadata = PluginBrowser::cache().metadata().find(item->fullName))
            gotPluginMetadata(entry, metadata);
        else
            m_browser->tryGetMetadata(item, entry);
    }
}

void PluginBrowserView::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
        close();
    else
        QWidget::keyPressEvent(event);
}

void PluginBrowserView::startPopulating()
{
    if (PluginRepoCache& reposCache = PluginBrowser::cache().repos(); !reposCache.isEmpty())
        gotRepositories(reposCache.data());
    else
        m_browser->getRepositories();
}

PluginBrowserViewEntry::PluginBrowserViewEntry(QWidget* parent)
    : BasePluginEntry(parent),
      m_installButton(new QPushButton("Install", this))
{
    m_installButton->setEnabled(false);
    m_buttonsLayout->addWidget(m_installButton);
    connect(m_installButton, &QPushButton::clicked, this, &PluginBrowserViewEntry::installButtonClicked);
}

void PluginBrowserViewEntry::setData(const PluginEntryMetadata& metadata)
{
    m_installButton->setEnabled(!qtTubeApp->plugins().containsPlugin(metadata.name));
    BasePluginEntry::setData(metadata);
}
