#include "settingsform.h"
#include "ui_settingsform.h"
#include "mainwindow.h"
#include "qttubeapplication.h"
#include "termfilterview.h"
#include "ui/forms/plugins/addplugindialog.h"
#include "ui/forms/plugins/pluginbrowserview.h"
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
    : QtTubePlugin::SettingsWindow(parent),
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
#endif

    SettingsStore& store = qtTubeApp->settings();
    // app style
    ui->appStyle->addItems(QStyleFactory::keys());
    ui->appStyle->setCurrentIndex(ui->appStyle->findText(store.appStyle));
    ui->darkTheme->setChecked(store.darkTheme);
    // general
    ui->autoHideTopBar->setChecked(store.autoHideTopBar);
    ui->imageCaching->setChecked(store.imageCaching);
    ui->preferLists->setChecked(store.preferLists);
    // player
    ui->externalPlayerEdit->setText(store.externalPlayerPath);
    ui->h264Only->setChecked(store.playerSettings.h264Only);
    ui->preferredQuality->setEnabled(!store.playerSettings.qualityFromPlayer);
    ui->preferredQuality->setCurrentIndex(static_cast<int>(store.playerSettings.preferredQuality));
    ui->preferredVolume->setEnabled(!store.playerSettings.volumeFromPlayer);
    ui->preferredVolume->setValue(store.playerSettings.preferredVolume);
    ui->qualityFromPlayer->setChecked(store.playerSettings.qualityFromPlayer);
    ui->vaapi->setChecked(store.playerSettings.vaapi);
    ui->vaapi->setEnabled(store.externalPlayerPath.isEmpty());
    ui->volumeFromPlayer->setChecked(store.playerSettings.volumeFromPlayer);
    // filtering
    ui->filterLength->setEnabled(store.filterLengthEnabled);
    ui->filterLength->setValue(store.filterLength);
    ui->filterLengthCheck->setChecked(store.filterLengthEnabled);

    connect(pluginActiveButtonGroup, &QButtonGroup::buttonToggled, this, &SettingsForm::pluginActiveButtonToggled);
    connect(ui->addPluginButton, &QPushButton::clicked, this, &SettingsForm::openAddPluginDialog);
    connect(ui->browsePluginsButton, &QPushButton::clicked, this, &SettingsForm::openPluginBrowser);
    connect(ui->clearCache, &QPushButton::clicked, this, &SettingsForm::clearCache);
    connect(ui->externalPlayerButton, &QPushButton::clicked, this, &SettingsForm::selectExternalPlayer);
    connect(ui->externalPlayerEdit, &QLineEdit::textEdited, this, &SettingsForm::checkExternalPlayer);
    connect(ui->filterLengthCheck, &QCheckBox::toggled, this, [this](bool c) { ui->filterLength->setEnabled(c); });
    connect(ui->qualityFromPlayer, &QCheckBox::toggled, this, [this](bool c) { ui->preferredQuality->setEnabled(!c); });
    connect(ui->showFilteredTerms, &QPushButton::clicked, this, &SettingsForm::openTermFilterTable);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &SettingsForm::currentChanged);
    connect(ui->volumeFromPlayer, &QCheckBox::toggled, this, [this](bool c) { ui->preferredVolume->setEnabled(!c); });

    setupSaveButton(ui->saveButton, true, { ui->addPluginButton, ui->browsePluginsButton });
}

void SettingsForm::checkExternalPlayer(const QString& text)
{
    if (text.isEmpty())
    {
        ui->externalPlayerEdit->setStyleSheet(QString());
        ui->vaapi->setEnabled(true);
        return;
    }

    if (!text.contains("%U"))
        return;

    if (QString playerPath = extractPath(text); QFileInfo(playerPath).isExecutable())
    {
        ui->externalPlayerEdit->setStyleSheet(QString());
        ui->vaapi->setEnabled(false);
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
        QMessageBox::warning(this, "No Cache Found", "The cache does not yet exist or is empty.");
        return;
    }

    directory.removeRecursively();
    QMessageBox::information(this, "Cache Cleared", "The cache was cleared successfully.");
}

void SettingsForm::currentChanged(int index)
{
    if (ui->tabWidget->tabText(index) != "Plugins" || !pluginActiveButtonGroup->buttons().isEmpty())
        return;

    for (PluginData* plugin : qtTubeApp->plugins().loadedPlugins())
    {
        AddPluginDialogEntry* entry = new AddPluginDialogEntry(plugin);
        UIUtils::addWidgetToList(ui->pluginsListWidget, entry);
        entry->setData(plugin->metadata);
        pluginActiveButtonGroup->addButton(entry->activeButton());
    }
}

QString SettingsForm::extractPath(const QString& str)
{
    QString out;
    quint8 quoteCount{};

    for (QChar c : str)
    {
        if (c == '"' || c == '\'')
            ++quoteCount;
        else if (c.isSpace() && quoteCount != 1)
            break;
        else
            out += c;
    }

    return out;
}

void SettingsForm::openAddPluginDialog()
{
    AddPluginDialog dialog;
    if (dialog.exec() == QDialog::Accepted)
    {
        PluginData* plugin = qtTubeApp->plugins().activePlugin();
        AddPluginDialogEntry* entry = new AddPluginDialogEntry(plugin);
        UIUtils::addWidgetToList(ui->pluginsListWidget, entry);
        entry->setData(plugin->metadata);
        pluginActiveButtonGroup->addButton(entry->activeButton());
    }
}

void SettingsForm::openPluginBrowser()
{
    PluginBrowserView::spawn();
}

void SettingsForm::openTermFilterTable()
{
    TermFilterView* fv = new TermFilterView;
    fv->show();
    fv->populateFromSettings();
}

void SettingsForm::pluginActiveButtonToggled(QAbstractButton* button, bool checked)
{
    if (AddPluginDialogEntry* entry = qobject_cast<AddPluginDialogEntry*>(button->parent()))
    {
        entry->data()->active = true;
        qtTubeApp->settings().activePlugin = entry->data()->fileInfo.fileName();
        ui->saveButton->setEnabled(true);
        emit qtTubeApp->activePluginChanged(entry->data());
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
    store.darkTheme = ui->darkTheme->isChecked();
    store.imageCaching = ui->imageCaching->isChecked();
    store.preferLists = ui->preferLists->isChecked();
    // player
    store.externalPlayerPath = ui->externalPlayerEdit->text();
    store.playerSettings.h264Only = ui->h264Only->isChecked();
    store.playerSettings.preferredQuality = static_cast<QtTubePlugin::PlayerSettings::Quality>(
        ui->preferredQuality->currentIndex());
    store.playerSettings.preferredVolume = ui->preferredVolume->value();
    store.playerSettings.qualityFromPlayer = ui->qualityFromPlayer->isChecked();
    store.playerSettings.vaapi = ui->vaapi->isChecked();
    store.playerSettings.volumeFromPlayer = ui->volumeFromPlayer->isChecked();
    // filtering
    store.filterLength = ui->filterLength->value();
    store.filterLengthEnabled = ui->filterLengthCheck->isChecked();

    store.save();
    store.init();

    UIUtils::setAppStyle(store.appStyle, store.darkTheme);
    QMessageBox::information(this, "Settings Saved", "Settings saved successfully.");
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
        ui->vaapi->setEnabled(false);
    }
}
