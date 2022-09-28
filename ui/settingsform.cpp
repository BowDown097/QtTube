#include "settingsform.h"
#include "ui_settingsform.h"
#include <QFileDialog>
#include <QMessageBox>

SettingsForm::SettingsForm(QWidget *parent) : QWidget(parent), ui(new Ui::SettingsForm)
{
    ui->setupUi(this);
    ui->condensedViews->setChecked(SettingsStore::instance().condensedViews);
    ui->itcCache->setChecked(SettingsStore::instance().itcCache);
    ui->playbackTracking->setChecked(SettingsStore::instance().playbackTracking);
    ui->volumeSpin->setValue(SettingsStore::instance().preferredVolume);
    ui->watchtimeTracking->setChecked(SettingsStore::instance().watchtimeTracking);
    connect(ui->saveButton, &QPushButton::clicked, this, &SettingsForm::saveSettings);
}

void SettingsForm::saveSettings()
{
    SettingsStore::instance().condensedViews = ui->condensedViews->isChecked();
    SettingsStore::instance().itcCache = ui->itcCache->isChecked();
    SettingsStore::instance().playbackTracking = ui->playbackTracking->isChecked();
    SettingsStore::instance().preferredVolume = ui->volumeSpin->value();
    SettingsStore::instance().watchtimeTracking = ui->watchtimeTracking->isChecked();
    SettingsStore::instance().saveToSettingsFile();
    SettingsStore::instance().initializeFromSettingsFile();
    QMessageBox::information(this, "Saved!", "Settings saved successfully.");
}

SettingsForm::~SettingsForm()
{
    delete ui;
}
