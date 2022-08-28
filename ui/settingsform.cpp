#include "settingsform.h"
#include "ui_settingsform.h"
#include <QFileDialog>
#include <QMessageBox>

SettingsForm::SettingsForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsForm)
{
    ui->setupUi(this);
    ui->argsText->setText(SettingsStore::instance().playerArgs);
    ui->itcCache->setChecked(SettingsStore::instance().itcCache);
    ui->playerText->setText(SettingsStore::instance().playerPath);

    if (!ui->playerText->text().isEmpty())
    {
        ui->argsLabel->setEnabled(true);
        ui->argsText->setEnabled(true);
    }

    connect(ui->playerSelect, &QPushButton::clicked, this, [this]() { ui->playerText->setText(QFileDialog::getOpenFileName(this, tr("Select video player"))); });
    connect(ui->saveButton, &QPushButton::clicked, this, &SettingsForm::saveSettings);
}

void SettingsForm::saveSettings()
{
    SettingsStore::instance().itcCache = ui->itcCache->isChecked();
    SettingsStore::instance().playerArgs = ui->argsText->text();
    SettingsStore::instance().playerPath = ui->playerText->text();
    SettingsStore::instance().saveToSettingsFile();
    SettingsStore::instance().initializeFromSettingsFile();
    QMessageBox::information(this, "Saved!", "Settings saved successfully.");
}

SettingsForm::~SettingsForm()
{
    delete ui;
}
