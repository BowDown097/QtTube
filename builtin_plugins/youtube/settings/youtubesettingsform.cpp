#include "youtubesettingsform.h"
#include "ui_youtubesettingsform.h"
#include "channelfiltertable.h"
#include "data-wizards/import/grayjayimportwizard.h"
#include "data-wizards/import/newpipeimportwizard.h"
#include "data-wizards/import/pipedimportwizard.h"
#include "data-wizards/import/takeoutimportwizard.h"
#include "youtubesettings.h"
#include <QMessageBox>

YouTubeSettingsForm::YouTubeSettingsForm(YouTubeSettings* settings, QWidget* parent)
    : QtTube::PluginSettingsWindow(parent), settings(settings), ui(new Ui::YouTubeSettingsForm)
{
    ui->setupUi(this);

    ui->takeoutRadio->setProperty("id", 1);
    ui->pipedRadio->setProperty("id", 2);
    ui->grayjayRadio->setProperty("id", 3);
    ui->newpipeRadio->setProperty("id", 4);

    // general
    ui->condensedCounts->setChecked(settings->condensedCounts);
    ui->fullSubs->setChecked(settings->fullSubs);
    ui->returnDislikes->setChecked(settings->returnDislikes);
    // player
    ui->blockAds->setChecked(settings->blockAds);
    ui->disable60Fps->setChecked(settings->disable60Fps);
    ui->disablePlayerInfoPanels->setChecked(settings->disablePlayerInfoPanels);
    ui->h264Only->setChecked(settings->h264Only);
    ui->preferredQuality->setEnabled(!settings->qualityFromPlayer);
    ui->preferredQuality->setCurrentIndex(static_cast<int>(settings->preferredQuality));
    ui->preferredVolume->setEnabled(!settings->volumeFromPlayer);
    ui->preferredVolume->setValue(settings->preferredVolume);
    ui->qualityFromPlayer->setChecked(settings->qualityFromPlayer);
    ui->restoreAnnotations->setChecked(settings->restoreAnnotations);
    ui->volumeFromPlayer->setChecked(settings->volumeFromPlayer);
    // privacy
    ui->playbackTracking->setChecked(settings->playbackTracking);
    ui->watchtimeTracking->setChecked(settings->watchtimeTracking);
    // filtering
    ui->hideSearchShelves->setChecked(settings->hideSearchShelves);
    ui->hideShorts->setChecked(settings->hideShorts);
    ui->hideStreams->setChecked(settings->hideStreams);
    // sponsorblock
    ui->blockFiller->setChecked(settings->sponsorBlockCategories.contains("filler"));
    ui->blockInteraction->setChecked(settings->sponsorBlockCategories.contains("interaction"));
    ui->blockIntro->setChecked(settings->sponsorBlockCategories.contains("intro"));
    ui->blockNonMusic->setChecked(settings->sponsorBlockCategories.contains("music_offtopic"));
    ui->blockOutro->setChecked(settings->sponsorBlockCategories.contains("outro"));
    ui->blockPreview->setChecked(settings->sponsorBlockCategories.contains("preview"));
    ui->blockSelfPromo->setChecked(settings->sponsorBlockCategories.contains("selfpromo"));
    ui->blockSponsor->setChecked(settings->sponsorBlockCategories.contains("sponsor"));
    ui->showSBToasts->setChecked(settings->showSBToasts);
    // dearrow
    ui->deArrow->setChecked(settings->deArrow);
    ui->deArrowThumbs->setChecked(settings->deArrowThumbs);
    ui->deArrowTitles->setChecked(settings->deArrowTitles);
    toggleDeArrowSettings(settings->deArrow);

    connect(ui->deArrow, &QCheckBox::toggled, this, &YouTubeSettingsForm::toggleDeArrowSettings);
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
    settings->condensedCounts = ui->condensedCounts->isChecked();
    settings->fullSubs = ui->fullSubs->isChecked();
    settings->returnDislikes = ui->returnDislikes->isChecked();
    // player
    settings->blockAds = ui->blockAds->isChecked();
    settings->disable60Fps = ui->disable60Fps->isChecked();
    settings->disablePlayerInfoPanels = ui->disablePlayerInfoPanels->isChecked();
    settings->h264Only = ui->h264Only->isChecked();
    settings->preferredQuality = static_cast<YouTubeSettings::PlayerQuality>(ui->preferredQuality->currentIndex());
    settings->preferredVolume = ui->preferredVolume->value();
    settings->qualityFromPlayer = ui->qualityFromPlayer->isChecked();
    settings->restoreAnnotations = ui->restoreAnnotations->isChecked();
    settings->volumeFromPlayer = ui->volumeFromPlayer->isChecked();
    // privacy
    settings->playbackTracking = ui->playbackTracking->isChecked();
    settings->watchtimeTracking = ui->watchtimeTracking->isChecked();
    // filtering
    settings->hideSearchShelves = ui->hideSearchShelves->isChecked();
    settings->hideShorts = ui->hideShorts->isChecked();
    settings->hideStreams = ui->hideStreams->isChecked();
    // sponsorblock
    handleSponsorCategory(settings->sponsorBlockCategories, "filler", ui->blockFiller);
    handleSponsorCategory(settings->sponsorBlockCategories, "interaction", ui->blockInteraction);
    handleSponsorCategory(settings->sponsorBlockCategories, "intro", ui->blockIntro);
    handleSponsorCategory(settings->sponsorBlockCategories, "music_offtopic", ui->blockNonMusic);
    handleSponsorCategory(settings->sponsorBlockCategories, "outro", ui->blockOutro);
    handleSponsorCategory(settings->sponsorBlockCategories, "preview", ui->blockPreview);
    handleSponsorCategory(settings->sponsorBlockCategories, "selfpromo", ui->blockSelfPromo);
    handleSponsorCategory(settings->sponsorBlockCategories, "sponsor", ui->blockSponsor);
    settings->showSBToasts = ui->showSBToasts->isChecked();
    // dearrow
    settings->deArrow = ui->deArrow->isChecked();
    settings->deArrowThumbs = ui->deArrowThumbs->isChecked();
    settings->deArrowTitles = ui->deArrowTitles->isChecked();

    settings->save();
    settings->init();

    QMessageBox::information(this, "Saved!", "Settings saved successfully.");
}

void YouTubeSettingsForm::showChannelFilterTable()
{
    ChannelFilterTable* ft = new ChannelFilterTable(settings);
    ft->show();
    ft->populateFromSettings();
}

void YouTubeSettingsForm::toggleDeArrowSettings(bool checked)
{
    ui->deArrowThumbs->setEnabled(checked);
    ui->deArrowTitles->setEnabled(checked);
}
