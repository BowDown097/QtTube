#include "settingsform.h"
#include "ui_settingsform.h"
#include "channelfiltertable.h"
#include "data-wizards/import/grayjayimportwizard.h"
#include "data-wizards/import/newpipeimportwizard.h"
#include "data-wizards/import/pipedimportwizard.h"
#include "data-wizards/import/takeoutimportwizard.h"
#include "qttubeapplication.h"
#include "termfilterview.h"
#include "utils/uiutils.h"
#include <QMessageBox>
#include <QStyleFactory>

SettingsForm::SettingsForm(QWidget* parent) : QWidget(parent), ui(new Ui::SettingsForm)
{
    ui->setupUi(this);

    ui->description->setText(QStringLiteral("%1<br>"
                                            "<a href=\"%2\">%2</a><br>"
                                            "Version: %3<br>"
                                            "Commit: %4 (%5)<br>"
                                            "Build date: %6")
                                 .arg(QTTUBE_APP_DESC, QTTUBE_REPO_URL, QTTUBE_VERSION_NAME, QTTUBE_COMMIT_ID, QTTUBE_BRANCH, __DATE__));
    ui->qttubeLogo->setPixmap(UIUtils::pixmapThemed("qttube-full", true, ui->qttubeLogo->size()));

    ui->takeoutRadio->setProperty("id", 1);
    ui->pipedRadio->setProperty("id", 2);
    ui->grayjayRadio->setProperty("id", 3);
    ui->newpipeRadio->setProperty("id", 4);

#ifndef Q_OS_LINUX
    ui->vaapi->setVisible(false);
#endif

    SettingsStore& store = qtTubeApp->settings();
    // app style
    ui->appStyle->addItems(QStyleFactory::keys());
    ui->appStyle->setCurrentIndex(ui->appStyle->findText(store.appStyle));
    ui->darkTheme->setChecked(store.darkTheme);
    // general
    ui->condensedCounts->setChecked(store.condensedCounts);
    ui->fullSubs->setChecked(store.fullSubs);
    ui->preferLists->setChecked(store.preferLists);
    ui->returnDislikes->setChecked(store.returnDislikes);
    // player
    ui->blockAds->setChecked(store.blockAds);
    ui->disable60Fps->setChecked(store.disable60Fps);
    ui->disablePlayerInfoPanels->setChecked(store.disablePlayerInfoPanels);
    ui->h264Only->setChecked(store.h264Only);
    ui->preferredQuality->setEnabled(!store.qualityFromPlayer);
    ui->preferredQuality->setCurrentIndex(static_cast<int>(store.preferredQuality));
    ui->preferredVolume->setEnabled(!store.volumeFromPlayer);
    ui->preferredVolume->setValue(store.preferredVolume);
    ui->qualityFromPlayer->setChecked(store.qualityFromPlayer);
    ui->restoreAnnotations->setChecked(store.restoreAnnotations);
    ui->vaapi->setChecked(store.vaapi);
    ui->volumeFromPlayer->setChecked(store.volumeFromPlayer);
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
    ui->showSBToasts->setChecked(store.showSBToasts);
    // dearrow
    ui->deArrow->setChecked(store.deArrow);
    ui->deArrowThumbs->setChecked(store.deArrowThumbs);
    ui->deArrowTitles->setChecked(store.deArrowTitles);
    toggleDeArrowSettings(store.deArrow);

    connect(ui->deArrow, &QCheckBox::toggled, this, &SettingsForm::toggleDeArrowSettings);
    //connect(ui->exportButton, &QPushButton::clicked, this, &SettingsForm::openExportWizard);
    connect(ui->filterLengthCheck, &QCheckBox::toggled, this, [this](bool checked) { ui->filterLength->setEnabled(checked); });
    connect(ui->importButton, &QPushButton::clicked, this, &SettingsForm::openImportWizard);
    connect(ui->qualityFromPlayer, &QCheckBox::toggled, this, [this](bool checked) { ui->preferredQuality->setEnabled(!checked); });
    connect(ui->saveButton, &QPushButton::clicked, this, &SettingsForm::saveSettings);
    connect(ui->showFilteredChannels, &QPushButton::clicked, this, &SettingsForm::showChannelFilterTable);
    connect(ui->showFilteredTerms, &QPushButton::clicked, this, &SettingsForm::showTermFilterTable);
    connect(ui->volumeFromPlayer, &QCheckBox::toggled, this, [this](bool checked) { ui->preferredVolume->setEnabled(!checked); });
}

/*
void SettingsForm::openExportWizard()
{
    const QList<QRadioButton*> radios = ui->groupBox->findChildren<QRadioButton*>();
    QRadioButton* selectedRadio = *std::ranges::find_if(radios, [](QRadioButton* radio) {
        return radio->isChecked();
    });

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
*/

void SettingsForm::openImportWizard()
{
    const QList<QRadioButton*> radios = ui->groupBox->findChildren<QRadioButton*>();
    QRadioButton* selectedRadio = *std::ranges::find_if(radios, [](QRadioButton* radio) {
        return radio->isChecked();
    });

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

void SettingsForm::saveSettings()
{
    SettingsStore& store = qtTubeApp->settings();
    // general
    store.appStyle = ui->appStyle->currentText();
    store.condensedCounts = ui->condensedCounts->isChecked();
    store.darkTheme = ui->darkTheme->isChecked();
    store.fullSubs = ui->fullSubs->isChecked();
    store.preferLists = ui->preferLists->isChecked();
    store.returnDislikes = ui->returnDislikes->isChecked();
    // player
    store.blockAds = ui->blockAds->isChecked();
    store.disable60Fps = ui->disable60Fps->isChecked();
    store.disablePlayerInfoPanels = ui->disablePlayerInfoPanels->isChecked();
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
    tryAddSponsorBlockCategory(store.sponsorBlockCategories, "filler", ui->blockFiller);
    tryAddSponsorBlockCategory(store.sponsorBlockCategories, "interaction", ui->blockInteraction);
    tryAddSponsorBlockCategory(store.sponsorBlockCategories, "intro", ui->blockIntro);
    tryAddSponsorBlockCategory(store.sponsorBlockCategories, "music_offtopic", ui->blockNonMusic);
    tryAddSponsorBlockCategory(store.sponsorBlockCategories, "outro", ui->blockOutro);
    tryAddSponsorBlockCategory(store.sponsorBlockCategories, "preview", ui->blockPreview);
    tryAddSponsorBlockCategory(store.sponsorBlockCategories, "selfpromo", ui->blockSelfPromo);
    tryAddSponsorBlockCategory(store.sponsorBlockCategories, "sponsor", ui->blockSponsor);
    store.showSBToasts = ui->showSBToasts->isChecked();
    // dearrow
    store.deArrow = ui->deArrow->isChecked();
    store.deArrowThumbs = ui->deArrowThumbs->isChecked();
    store.deArrowTitles = ui->deArrowTitles->isChecked();

    store.save();
    store.initialize();

    UIUtils::setAppStyle(store.appStyle, store.darkTheme);
    QMessageBox::information(this, "Saved!", "Settings saved successfully.");
}

void SettingsForm::showChannelFilterTable()
{
    ChannelFilterTable* ft = new ChannelFilterTable;
    ft->show();
    ft->populateFromSettings();
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

void SettingsForm::tryAddSponsorBlockCategory(QStringList& sponsorBlockCategories, const QString& category,
                                              QCheckBox* checkBox)
{
    if (checkBox->isChecked() && !sponsorBlockCategories.contains(category))
        sponsorBlockCategories.append(category);
}

SettingsForm::~SettingsForm()
{
    delete ui;
}
