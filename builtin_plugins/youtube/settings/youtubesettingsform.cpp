#include "youtubesettingsform.h"
#include "ui_youtubesettingsform.h"
#include "channelfiltertable.h"
#include "data-wizards/import/grayjayimportwizard.h"
#include "data-wizards/import/newpipeimportwizard.h"
#include "data-wizards/import/pipedimportwizard.h"
#include "data-wizards/import/takeoutimportwizard.h"
#include "youtubeplugin.h"
#include <QMessageBox>

YouTubeSettingsForm::YouTubeSettingsForm(QWidget* parent)
    : QtTubePlugin::SettingsWindow(parent), ui(new Ui::YouTubeSettingsForm)
{
    ui->setupUi(this);

    ui->takeoutRadio->setProperty("id", 1);
    ui->pipedRadio->setProperty("id", 2);
    ui->grayjayRadio->setProperty("id", 3);
    ui->newpipeRadio->setProperty("id", 4);

    // general
    ui->condensedCounts->setChecked(g_settings->condensedCounts);
    ui->fullSubs->setChecked(g_settings->fullSubs);
    ui->returnDislikes->setChecked(g_settings->returnDislikes);
    // player
    ui->blockAds->setChecked(g_settings->blockAds);
    ui->disable60Fps->setChecked(g_settings->disable60Fps);
    ui->disablePlayerInfoPanels->setChecked(g_settings->disablePlayerInfoPanels);
    ui->h264Only->setChecked(g_settings->h264Only);
    ui->preferredQuality->setEnabled(!g_settings->qualityFromPlayer);
    ui->preferredQuality->setCurrentIndex(static_cast<int>(g_settings->preferredQuality));
    ui->preferredVolume->setEnabled(!g_settings->volumeFromPlayer);
    ui->preferredVolume->setValue(g_settings->preferredVolume);
    ui->qualityFromPlayer->setChecked(g_settings->qualityFromPlayer);
    ui->restoreAnnotations->setChecked(g_settings->restoreAnnotations);
    ui->volumeFromPlayer->setChecked(g_settings->volumeFromPlayer);
    // privacy
    ui->playbackTracking->setChecked(g_settings->playbackTracking);
    ui->watchtimeTracking->setChecked(g_settings->watchtimeTracking);
    // filtering
    ui->hideSearchShelves->setChecked(g_settings->hideSearchShelves);
    ui->hideShorts->setChecked(g_settings->hideShorts);
    ui->hideStreams->setChecked(g_settings->hideStreams);
    // sponsorblock
    ui->blockFiller->setChecked(g_settings->sponsorBlockCategories.contains("filler"));
    ui->blockInteraction->setChecked(g_settings->sponsorBlockCategories.contains("interaction"));
    ui->blockIntro->setChecked(g_settings->sponsorBlockCategories.contains("intro"));
    ui->blockNonMusic->setChecked(g_settings->sponsorBlockCategories.contains("music_offtopic"));
    ui->blockOutro->setChecked(g_settings->sponsorBlockCategories.contains("outro"));
    ui->blockPreview->setChecked(g_settings->sponsorBlockCategories.contains("preview"));
    ui->blockSelfPromo->setChecked(g_settings->sponsorBlockCategories.contains("selfpromo"));
    ui->blockSponsor->setChecked(g_settings->sponsorBlockCategories.contains("sponsor"));
    ui->showSBToasts->setChecked(g_settings->showSBToasts);

    //connect(ui->exportButton, &QPushButton::clicked, this, &YouTubeSettingsForm::openExportWizard);
    connect(ui->importButton, &QPushButton::clicked, this, &YouTubeSettingsForm::openImportWizard);
    connect(ui->qualityFromPlayer, &QCheckBox::toggled, this, [this](bool c) { ui->preferredQuality->setEnabled(!c); });
    connect(ui->showFilteredChannels, &QPushButton::clicked, this, &YouTubeSettingsForm::showChannelFilterTable);
    connect(ui->volumeFromPlayer, &QCheckBox::toggled, this, [this](bool c) { ui->preferredVolume->setEnabled(!c); });

    setupSaveButton(ui->saveButton, true, {
        ui->saveButton,
        ui->importButton,
        ui->exportButton,
        ui->grayjayRadio,
        ui->newpipeRadio,
        ui->pipedRadio,
        ui->takeoutRadio
    });
}

YouTubeSettingsForm::~YouTubeSettingsForm()
{
    delete ui;
}

void YouTubeSettingsForm::handleSponsorCategory(QStringList& categories, const QString& category, QCheckBox* checkBox)
{
    int categoryIndex = categories.indexOf(category);
    if (checkBox->isChecked() && categoryIndex == -1)
        categories.append(category);
    else if (!checkBox->isChecked() && categoryIndex != -1)
        categories.removeAt(categoryIndex);
}

/*
void YouTubeSettingsForm::openExportWizard()
{
    const QList<QRadioButton*> radios = ui->groupBox->findChildren<QRadioButton*>();
    if (auto it = std::ranges::find_if(radios, &QRadioButton::isChecked); it != radios.end())
    {
        QRadioButton* selectedRadio = *it;
        switch (selectedRadio->property("id").toInt())
        {
        case 1:
            TakeoutExportWizard().exec();
            break;
        case 2:
            PipedExportWizard().exec();
            break;
        case 3:
            GrayjayExportWizard().exec();
            break;
        case 4:
            NewPipeExportWizard().exec();
            break;
        }
    }
}
*/

void YouTubeSettingsForm::openImportWizard()
{
    const QList<QRadioButton*> radios = ui->dataSourcesGroup->findChildren<QRadioButton*>();
    if (auto it = std::ranges::find_if(radios, &QRadioButton::isChecked); it != radios.end())
    {
        QRadioButton* selectedRadio = *it;
        switch (selectedRadio->property("id").toInt())
        {
        case 1:
            TakeoutImportWizard().exec();
            break;
        case 2:
            PipedImportWizard().exec();
            break;
        case 3:
            GrayjayImportWizard().exec();
            break;
        case 4:
            NewPipeImportWizard().exec();
            break;
        }
    }
}

bool YouTubeSettingsForm::savePending() const
{
    return ui->saveButton->isEnabled();
}

void YouTubeSettingsForm::saveSettings()
{
    // general
    g_settings->condensedCounts = ui->condensedCounts->isChecked();
    g_settings->fullSubs = ui->fullSubs->isChecked();
    g_settings->returnDislikes = ui->returnDislikes->isChecked();
    // player
    g_settings->blockAds = ui->blockAds->isChecked();
    g_settings->disable60Fps = ui->disable60Fps->isChecked();
    g_settings->disablePlayerInfoPanels = ui->disablePlayerInfoPanels->isChecked();
    g_settings->h264Only = ui->h264Only->isChecked();
    g_settings->preferredQuality = static_cast<YouTubeSettings::PlayerQuality>(ui->preferredQuality->currentIndex());
    g_settings->preferredVolume = ui->preferredVolume->value();
    g_settings->qualityFromPlayer = ui->qualityFromPlayer->isChecked();
    g_settings->restoreAnnotations = ui->restoreAnnotations->isChecked();
    g_settings->volumeFromPlayer = ui->volumeFromPlayer->isChecked();
    // privacy
    g_settings->playbackTracking = ui->playbackTracking->isChecked();
    g_settings->watchtimeTracking = ui->watchtimeTracking->isChecked();
    // filtering
    g_settings->hideSearchShelves = ui->hideSearchShelves->isChecked();
    g_settings->hideShorts = ui->hideShorts->isChecked();
    g_settings->hideStreams = ui->hideStreams->isChecked();
    // sponsorblock
    handleSponsorCategory(g_settings->sponsorBlockCategories, "filler", ui->blockFiller);
    handleSponsorCategory(g_settings->sponsorBlockCategories, "interaction", ui->blockInteraction);
    handleSponsorCategory(g_settings->sponsorBlockCategories, "intro", ui->blockIntro);
    handleSponsorCategory(g_settings->sponsorBlockCategories, "music_offtopic", ui->blockNonMusic);
    handleSponsorCategory(g_settings->sponsorBlockCategories, "outro", ui->blockOutro);
    handleSponsorCategory(g_settings->sponsorBlockCategories, "preview", ui->blockPreview);
    handleSponsorCategory(g_settings->sponsorBlockCategories, "selfpromo", ui->blockSelfPromo);
    handleSponsorCategory(g_settings->sponsorBlockCategories, "sponsor", ui->blockSponsor);
    g_settings->showSBToasts = ui->showSBToasts->isChecked();

    g_settings->save();
    g_settings->init();

    QMessageBox::information(this, "Saved!", "Settings saved successfully.");
}

void YouTubeSettingsForm::showChannelFilterTable()
{
    ChannelFilterTable* ft = new ChannelFilterTable;
    ft->show();
    ft->populateFromSettings();
}
