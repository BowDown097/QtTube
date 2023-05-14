#include "settingsform.h"
#include "ui_settingsform.h"
#include "settingsstore.h"
#include "ui/uiutilities.h"
#include <QMessageBox>
#include <QStyleFactory>

SettingsForm::SettingsForm(QWidget *parent) : QWidget(parent), ui(new Ui::SettingsForm)
{
    ui->setupUi(this);

    SettingsStore& store = SettingsStore::instance();
    // app style
    ui->appStyle->addItems(QStyleFactory::keys());
    ui->appStyle->setCurrentIndex(ui->appStyle->findText(store.appStyle));
#ifndef Q_OS_WIN
    ui->darkThemeWindows->setVisible(false);
#else
    ui->darkThemeWindows->setChecked(store.darkThemeWindows);
#endif
    // general
    ui->condensedViews->setChecked(store.condensedViews);
    ui->fullSubs->setChecked(store.fullSubs);
    ui->homeShelves->setChecked(store.homeShelves);
    ui->returnDislikes->setChecked(store.returnDislikes);
    ui->themedChannels->setChecked(store.themedChannels);
    // player
    ui->disable60Fps->setChecked(store.disable60Fps);
    ui->disablePlayerInfoPanels->setChecked(store.disablePlayerInfoPanels);
    ui->h264Only->setChecked(store.h264Only);
    ui->preferredQuality->setCurrentIndex(store.preferredQuality);
    ui->preferredVolume->setValue(store.preferredVolume);
    ui->restoreAnnotations->setChecked(store.restoreAnnotations);
    // privacy
    ui->playbackTracking->setChecked(store.playbackTracking);
    ui->watchtimeTracking->setChecked(store.watchtimeTracking);
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

    connect(ui->saveButton, &QPushButton::clicked, this, &SettingsForm::saveSettings);
}

void SettingsForm::saveSettings()
{
    SettingsStore& store = SettingsStore::instance();
    // general
    store.appStyle = ui->appStyle->currentText();
    store.condensedViews = ui->condensedViews->isChecked();
    store.fullSubs = ui->fullSubs->isChecked();
    store.homeShelves = ui->homeShelves->isChecked();
    store.returnDislikes = ui->returnDislikes->isChecked();
    store.themedChannels = ui->themedChannels->isChecked();
#ifdef Q_OS_WIN
    store.darkThemeWindows = ui->darkThemeWindows->isChecked();
#endif
    // player
    store.disable60Fps = ui->disable60Fps->isChecked();
    store.disablePlayerInfoPanels = ui->disablePlayerInfoPanels->isChecked();
    store.h264Only = ui->h264Only->isChecked();
    store.preferredQuality = static_cast<SettingsStore::PlayerQuality>(ui->preferredQuality->currentIndex());
    store.preferredVolume = ui->preferredVolume->value();
    store.restoreAnnotations = ui->restoreAnnotations->isChecked();
    // privacy
    store.playbackTracking = ui->playbackTracking->isChecked();
    store.watchtimeTracking = ui->watchtimeTracking->isChecked();
    // sponsorblock
    if (ui->blockFiller->isChecked() && !store.sponsorBlockCategories.contains("filler"))
        store.sponsorBlockCategories.append("filler");
    if (ui->blockInteraction->isChecked() && !store.sponsorBlockCategories.contains("interaction"))
        store.sponsorBlockCategories.append("interaction");
    if (ui->blockIntro->isChecked() && !store.sponsorBlockCategories.contains("intro"))
        store.sponsorBlockCategories.append("intro");
    if (ui->blockNonMusic->isChecked() && !store.sponsorBlockCategories.contains("music_offtopic"))
        store.sponsorBlockCategories.append("music_offtopic");
    if (ui->blockOutro->isChecked() && !store.sponsorBlockCategories.contains("outro"))
        store.sponsorBlockCategories.append("outro");
    if (ui->blockPreview->isChecked() && !store.sponsorBlockCategories.contains("preview"))
        store.sponsorBlockCategories.append("preview");
    if (ui->blockSelfPromo->isChecked() && !store.sponsorBlockCategories.contains("selfpromo"))
        store.sponsorBlockCategories.append("selfpromo");
    if (ui->blockSponsor->isChecked() && !store.sponsorBlockCategories.contains("sponsor"))
        store.sponsorBlockCategories.append("sponsor");
    store.showSBToasts = ui->showSBToasts->isChecked();

    store.saveToSettingsFile();
    store.initializeFromSettingsFile();

#ifdef Q_OS_WIN
    UIUtilities::setAppStyle(store.appStyle, store.darkThemeWindows);
#else
    UIUtilities::setAppStyle(store.appStyle);
#endif

    QMessageBox::information(this, "Saved!", "Settings saved successfully.");
}

SettingsForm::~SettingsForm()
{
    delete ui;
}
