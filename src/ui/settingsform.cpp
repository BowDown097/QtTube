#include "../settingsstore.h"
#include "settingsform.h"
#include "ui_settingsform.h"
#include <QMessageBox>

SettingsForm::SettingsForm(QWidget *parent) : QWidget(parent), ui(new Ui::SettingsForm)
{
    ui->setupUi(this);

    SettingsStore& store = SettingsStore::instance();
    // general
    ui->itcCache->setChecked(store.itcCache);
    ui->returnDislike->setChecked(store.returnDislikes);
    ui->fullSubs->setChecked(store.fullSubs);
    ui->condensedViews->setChecked(store.condensedViews);
    ui->homeShelves->setChecked(store.homeShelves);
    ui->volumeSpin->setValue(store.preferredVolume);
    // privacy
    ui->playbackTracking->setChecked(store.playbackTracking);
    ui->watchtimeTracking->setChecked(store.watchtimeTracking);
    // sponsorblock
    ui->blockSponsor->setChecked(store.sponsorBlockCategories.contains("sponsor"));
    ui->blockIntro->setChecked(store.sponsorBlockCategories.contains("intro"));
    ui->blockOutro->setChecked(store.sponsorBlockCategories.contains("outro"));
    ui->blockSelfPromo->setChecked(store.sponsorBlockCategories.contains("selfpromo"));
    ui->blockPreview->setChecked(store.sponsorBlockCategories.contains("preview"));
    ui->blockFiller->setChecked(store.sponsorBlockCategories.contains("filler"));
    ui->blockInteraction->setChecked(store.sponsorBlockCategories.contains("interaction"));
    ui->blockNonMusic->setChecked(store.sponsorBlockCategories.contains("music_offtopic"));
    ui->showToasts->setChecked(store.showSBToasts);

    connect(ui->saveButton, &QPushButton::clicked, this, &SettingsForm::saveSettings);
}

void SettingsForm::saveSettings()
{
    SettingsStore& store = SettingsStore::instance();
    // general
    store.itcCache = ui->itcCache->isChecked();
    store.returnDislikes = ui->returnDislike->isChecked();
    store.fullSubs = ui->fullSubs->isChecked();
    store.condensedViews = ui->condensedViews->isChecked();
    store.homeShelves = ui->homeShelves->isChecked();
    store.preferredVolume = ui->volumeSpin->value();
    // privacy
    store.playbackTracking = ui->playbackTracking->isChecked();
    store.watchtimeTracking = ui->watchtimeTracking->isChecked();
    // sponsorblock
    if (ui->blockSponsor->isChecked()) store.sponsorBlockCategories.append("sponsor");
    if (ui->blockIntro->isChecked()) store.sponsorBlockCategories.append("intro");
    if (ui->blockOutro->isChecked()) store.sponsorBlockCategories.append("outro");
    if (ui->blockSelfPromo->isChecked()) store.sponsorBlockCategories.append("selfpromo");
    if (ui->blockPreview->isChecked()) store.sponsorBlockCategories.append("preview");
    if (ui->blockFiller->isChecked()) store.sponsorBlockCategories.append("filler");
    if (ui->blockInteraction->isChecked()) store.sponsorBlockCategories.append("interaction");
    if (ui->blockNonMusic->isChecked()) store.sponsorBlockCategories.append("music_offtopic");
    store.showSBToasts = ui->showToasts->isChecked();

    store.saveToSettingsFile();
    store.initializeFromSettingsFile();
    QMessageBox::information(this, "Saved!", "Settings saved successfully.");
}

SettingsForm::~SettingsForm()
{
    delete ui;
}
