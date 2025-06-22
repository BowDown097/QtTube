#include "settingsform.h"
#include "ui_settingsform.h"
#include "mainwindow.h"
#include "qttubeapplication.h"
#include "termfilterview.h"
#include "ui/widgets/pluginwidget.h"
#include "utils/stringutils.h"
#include "utils/uiutils.h"
#include <QButtonGroup>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QStyleFactory>

constexpr QLatin1String DescriptionTemplate(R"(
%1<br>
<a href=\"%2\">%2</a><br>
Version: %3<br>
Commit: %4 (%5)<br>
Build date: %6
)");

SettingsForm::~SettingsForm() { delete ui; }

SettingsForm::SettingsForm(QWidget* parent)
    : QtTube::PluginSettingsWindow(parent),
      pluginActiveButtonGroup(new QButtonGroup(this)),
      ui(new Ui::SettingsForm)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    ui->description->setText(DescriptionTemplate
        .arg(QTTUBE_APP_DESC, QTTUBE_REPO_URL, QTTUBE_VERSION_NAME, QTTUBE_COMMIT_ID, QTTUBE_BRANCH, __DATE__));
    ui->qttubeLogo->setPixmap(UIUtils::iconThemed("qttube-full").pixmap(ui->qttubeLogo->size()));

#ifndef Q_OS_LINUX
    ui->vaapi->hide();
#else
    ui->vaapi->setVisible(qApp->platformName() != "wayland");
#endif

    SettingsStore& store = qtTubeApp->settings();
    // app style
    ui->appStyle->addItems(QStyleFactory::keys());
    ui->appStyle->setCurrentIndex(ui->appStyle->findText(store.appStyle));
    ui->darkTheme->setChecked(store.darkTheme);
    // general
    ui->autoHideTopBar->setChecked(store.autoHideTopBar);
    ui->condensedCounts->setChecked(store.condensedCounts);
    ui->fullSubs->setChecked(store.fullSubs);
    ui->imageCaching->setChecked(store.imageCaching);
    ui->preferLists->setChecked(store.preferLists);
    // player
    ui->blockAds->setChecked(store.blockAds);
    ui->disable60Fps->setChecked(store.disable60Fps);
    ui->disablePlayerInfoPanels->setChecked(store.disablePlayerInfoPanels);
    ui->externalPlayerEdit->setText(store.externalPlayerPath);
    ui->h264Only->setChecked(store.h264Only);
    ui->preferredQuality->setEnabled(!store.qualityFromPlayer);
    ui->preferredQuality->setCurrentIndex(static_cast<int>(store.preferredQuality));
    ui->preferredVolume->setEnabled(!store.volumeFromPlayer);
    ui->preferredVolume->setValue(store.preferredVolume);
    ui->qualityFromPlayer->setChecked(store.qualityFromPlayer);
    ui->restoreAnnotations->setChecked(store.restoreAnnotations);
    ui->vaapi->setChecked(store.vaapi);
    ui->volumeFromPlayer->setChecked(store.volumeFromPlayer);
    toggleWebPlayerSettings(store.externalPlayerPath.isEmpty());
    // privacy
    ui->playbackTracking->setChecked(store.playbackTracking);
    ui->watchtimeTracking->setChecked(store.watchtimeTracking);
    // filtering
    ui->filterLength->setEnabled(store.filterLengthEnabled);
    ui->filterLength->setValue(store.filterLength);
    ui->filterLengthCheck->setChecked(store.filterLengthEnabled);
    ui->hideShorts->setChecked(store.hideShorts);
    ui->hideStreams->setChecked(store.hideStreams);
    // sponsorblock
    ui->blockFiller->setChecked(store.sponsorBlockCategories.contains("filler"));
    ui->blockInteraction->setChecked(store.sponsorBlockCategories.contains("interaction"));
    ui->blockIntro->setChecked(store.sponsorBlockCategories.contains("intro"));
    ui->blockNonMusic->setChecked(store.sponsorBlockCategories.contains("music_offtopic"));
    ui->blockOutro->setChecked(store.sponsorBlockCategories.contains("outro"));
    ui->blockPreview->setChecked(store.sponsorBlockCategories.contains("preview"));
    ui->blockSelfPromo->setChecked(store.sponsorBlockCategories.contains("selfpromo"));
    ui->blockSponsor->setChecked(store.sponsorBlockCategories.contains("sponsor"));
    // dearrow
    ui->deArrow->setChecked(store.deArrow);
    ui->deArrowThumbs->setChecked(store.deArrowThumbs);
    ui->deArrowTitles->setChecked(store.deArrowTitles);
    toggleDeArrowSettings(store.deArrow);

    connect(pluginActiveButtonGroup, &QButtonGroup::buttonToggled, this, &SettingsForm::pluginActiveButtonToggled);
    connect(ui->clearCache, &QPushButton::clicked, this, &SettingsForm::clearCache);
    connect(ui->deArrow, &QCheckBox::toggled, this, &SettingsForm::toggleDeArrowSettings);
    connect(ui->externalPlayerButton, &QPushButton::clicked, this, &SettingsForm::selectExternalPlayer);
    connect(ui->externalPlayerEdit, &QLineEdit::textEdited, this, &SettingsForm::checkExternalPlayer);
    connect(ui->filterLengthCheck, &QCheckBox::toggled, this, [this](bool c) { ui->filterLength->setEnabled(c); });
    connect(ui->qualityFromPlayer, &QCheckBox::toggled, this, [this](bool c) { ui->preferredQuality->setEnabled(!c); });
    connect(ui->showFilteredTerms, &QPushButton::clicked, this, &SettingsForm::showTermFilterTable);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &SettingsForm::currentChanged);
    connect(ui->volumeFromPlayer, &QCheckBox::toggled, this, [this](bool c) { ui->preferredVolume->setEnabled(!c); });

    setupSaveButton(ui->saveButton, true);
}

void SettingsForm::checkExternalPlayer(const QString& text)
{
    if (text.isEmpty())
    {
        ui->externalPlayerEdit->setStyleSheet(QString());
        toggleWebPlayerSettings(true);
        return;
    }

    if (!text.contains("%U"))
        return;

    if (QString playerPath = StringUtils::extractPath(text); QFileInfo(playerPath).isExecutable())
    {
        ui->externalPlayerEdit->setStyleSheet(QString());
        toggleWebPlayerSettings(false);
    }
    else
    {
        ui->externalPlayerEdit->setStyleSheet("background-color: red");
    }
}

void SettingsForm::clearCache()
{
    QDir directory(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/http/");
    if (!directory.exists() || directory.isEmpty())
    {
        QMessageBox::warning(this, "No cache directory", "No cache directory exists.");
        return;
    }

    directory.removeRecursively();
    QMessageBox::information(this, "Cleared", "Cache directory cleared successfully.");
}

void SettingsForm::currentChanged(int index)
{
    if (ui->tabWidget->tabText(index) != "Plugins" || !pluginActiveButtonGroup->buttons().isEmpty())
        return;

    if (const QString& activePluginSetting = qtTubeApp->settings().activePlugin; !activePluginSetting.isEmpty())
    {
        if (PluginData* plugin = qtTubeApp->plugins().findPlugin(activePluginSetting))
        {
            if (PluginData* activePlugin = qtTubeApp->plugins().activePlugin(); activePlugin && activePlugin != plugin)
                activePlugin->active = false;
            plugin->active = true;
        }
    }

    for (PluginData* plugin : qtTubeApp->plugins().loadedPlugins())
    {
        PluginWidget* pluginWidget = new PluginWidget(plugin);
        UIUtils::addWidgetToList(ui->pluginsListWidget, pluginWidget);
        pluginActiveButtonGroup->addButton(pluginWidget->activeButton());
    }
}

void SettingsForm::handleSponsorCategory(QStringList& categories, const QString& category, QCheckBox* checkBox)
{
    int categoryIndex = categories.indexOf(category);
    if (checkBox->isChecked() && categoryIndex == -1)
        categories.append(category);
    else if (!checkBox->isChecked() && categoryIndex != -1)
        categories.removeAt(categoryIndex);
}

void SettingsForm::pluginActiveButtonToggled(QAbstractButton* button, bool checked)
{
    if (PluginWidget* pluginWidget = qobject_cast<PluginWidget*>(button->parent()))
    {
        pluginWidget->data()->active = true;
        qtTubeApp->settings().activePlugin = pluginWidget->data()->fileInfo.fileName();
    }
}

bool SettingsForm::savePending() const
{
    return ui->saveButton->isEnabled();
}

void SettingsForm::saveSettings()
{
    SettingsStore& store = qtTubeApp->settings();

    // force show top bar if auto hiding has been turned off
    if (store.autoHideTopBar && !ui->autoHideTopBar->isChecked())
        MainWindow::topbar()->show();

    // general
    store.appStyle = ui->appStyle->currentText();
    store.autoHideTopBar = ui->autoHideTopBar->isChecked();
    store.condensedCounts = ui->condensedCounts->isChecked();
    store.darkTheme = ui->darkTheme->isChecked();
    store.fullSubs = ui->fullSubs->isChecked();
    store.imageCaching = ui->imageCaching->isChecked();
    store.preferLists = ui->preferLists->isChecked();
    // player
    store.blockAds = ui->blockAds->isChecked();
    store.disable60Fps = ui->disable60Fps->isChecked();
    store.disablePlayerInfoPanels = ui->disablePlayerInfoPanels->isChecked();
    store.externalPlayerPath = ui->externalPlayerEdit->text();
    store.h264Only = ui->h264Only->isChecked();
    store.preferredQuality = static_cast<SettingsStore::PlayerQuality>(ui->preferredQuality->currentIndex());
    store.preferredVolume = ui->preferredVolume->value();
    store.qualityFromPlayer = ui->qualityFromPlayer->isChecked();
    store.restoreAnnotations = ui->restoreAnnotations->isChecked();
    store.vaapi = ui->vaapi->isChecked();
    store.volumeFromPlayer = ui->volumeFromPlayer->isChecked();
    // privacy
    store.playbackTracking = ui->playbackTracking->isChecked();
    store.watchtimeTracking = ui->watchtimeTracking->isChecked();
    // filtering
    store.filterLength = ui->filterLength->value();
    store.filterLengthEnabled = ui->filterLengthCheck->isChecked();
    store.hideShorts = ui->hideShorts->isChecked();
    store.hideStreams = ui->hideStreams->isChecked();
    // sponsorblock
    handleSponsorCategory(store.sponsorBlockCategories, "filler", ui->blockFiller);
    handleSponsorCategory(store.sponsorBlockCategories, "interaction", ui->blockInteraction);
    handleSponsorCategory(store.sponsorBlockCategories, "intro", ui->blockIntro);
    handleSponsorCategory(store.sponsorBlockCategories, "music_offtopic", ui->blockNonMusic);
    handleSponsorCategory(store.sponsorBlockCategories, "outro", ui->blockOutro);
    handleSponsorCategory(store.sponsorBlockCategories, "preview", ui->blockPreview);
    handleSponsorCategory(store.sponsorBlockCategories, "selfpromo", ui->blockSelfPromo);
    handleSponsorCategory(store.sponsorBlockCategories, "sponsor", ui->blockSponsor);
    // dearrow
    store.deArrow = ui->deArrow->isChecked();
    store.deArrowThumbs = ui->deArrowThumbs->isChecked();
    store.deArrowTitles = ui->deArrowTitles->isChecked();

    store.save();
    store.initialize();

    UIUtils::setAppStyle(store.appStyle, store.darkTheme);
    QMessageBox::information(this, "Saved!", "Settings saved successfully.");
}

void SettingsForm::selectExternalPlayer()
{
#if defined(Q_OS_MACOS)
    QString dir = "/Applications";
#elif defined(Q_OS_UNIX)
    QString dir = "/usr/bin";
#elif defined(Q_OS_WIN)
    QString dir = "C:\\Program Files";
#else
    QString dir;
#endif

    if (QString playerPath = QFileDialog::getOpenFileName(this, QString(), dir); !playerPath.isNull())
    {
        ui->externalPlayerEdit->setText(playerPath + " %U");
        toggleWebPlayerSettings(false);
    }
}

void SettingsForm::showTermFilterTable()
{
    TermFilterView* fv = new TermFilterView;
    fv->show();
    fv->populateFromSettings();
}

void SettingsForm::toggleDeArrowSettings(bool checked)
{
    ui->deArrowThumbs->setEnabled(checked);
    ui->deArrowTitles->setEnabled(checked);
}

void SettingsForm::toggleWebPlayerSettings(bool checked)
{
    ui->blockAds->setEnabled(checked);
    ui->disable60Fps->setEnabled(checked);
    ui->disablePlayerInfoPanels->setEnabled(checked);
    ui->h264Only->setEnabled(checked);
    ui->preferredQuality->setEnabled(checked);
    ui->preferredQualityLabel->setEnabled(checked);
    ui->preferredVolume->setEnabled(checked);
    ui->preferredVolumeLabel->setEnabled(checked);
    ui->restoreAnnotations->setEnabled(checked);
    ui->vaapi->setEnabled(checked);
}
