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

void PluginBrowserView::downloadBuild(BasePluginEntry* entry, const ReleaseData& data)
{
    PluginBuildDownloader* downloader = new PluginBuildDownloader(data);
    downloader->show();
    connect(downloader, &PluginBuildDownloader::success, this, [entry] {
        static_cast<PluginBrowserViewEntry*>(entry)->m_installButton->setEnabled(false);
    });
}

void PluginBrowserView::error(const QString& context, const QString& message)
{
    QMessageBox::critical(this, "Failed " + context, message);
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
    connect(viewEntry->m_installButton, &QPushButton::clicked, this, [this, entry, metadata] {
        m_browser->getReleaseData(entry, metadata);
    });
}

void PluginBrowserView::gotReleaseData(
    BasePluginEntry* entry,
    const PluginEntryMetadataPtr& metadata,
    const std::optional<ReleaseData>& data)
{
    if (!data)
    {
        QMessageBox::StandardButton button = QMessageBox::question(
            this, "No Release Found",
            "No release has been found for this plugin. Would you like to try to get a nightly build?");
        if (button == QMessageBox::Yes)
            m_browser->getNightlyBuild(entry, metadata);
        return;
    }

    downloadBuild(entry, data.value());
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
            m_browser->getMetadata(entry, item);
    }
}

void PluginBrowserView::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
        close();
    else
        QWidget::keyPressEvent(event);
}

PluginBrowserView* PluginBrowserView::spawn()
{
    if (!qtTubeApp->isSelfContainedBuild() && QMessageBox::warning(nullptr, "Warning",
            "Installing plugins from the plugin browser is not supported in this build "
            "because it is not self-contained. You will have to build plugins yourself. "
            "Do you wish to continue?",
            QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
    {
        return nullptr;
    }

    PluginBrowserView* inst = new PluginBrowserView;
    inst->show();
    inst->startPopulating();
    return inst;
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
}

void PluginBrowserViewEntry::setData(const PluginEntryMetadata& metadata)
{
    m_installButton->setEnabled(
        qtTubeApp->isSelfContainedBuild() &&!qtTubeApp->plugins().containsPlugin(metadata.name));
    BasePluginEntry::setData(metadata);
}
