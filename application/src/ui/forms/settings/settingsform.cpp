#include "settingsform.h"
#include "ui_settingsform.h"
#include "mainwindow.h"
#include "qttubeapplication.h"
#include "termfilterview.h"
#include "ui/widgets/pluginwidget.h"
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
    ui->vaapi->setChecked(store.vaapi);
    ui->vaapi->setEnabled(store.externalPlayerPath.isEmpty());
    // filtering
    ui->filterLength->setEnabled(store.filterLengthEnabled);
    ui->filterLength->setValue(store.filterLength);
    ui->filterLengthCheck->setChecked(store.filterLengthEnabled);

    connect(pluginActiveButtonGroup, &QButtonGroup::buttonToggled, this, &SettingsForm::pluginActiveButtonToggled);
    connect(ui->clearCache, &QPushButton::clicked, this, &SettingsForm::clearCache);
    connect(ui->externalPlayerButton, &QPushButton::clicked, this, &SettingsForm::selectExternalPlayer);
    connect(ui->externalPlayerEdit, &QLineEdit::textEdited, this, &SettingsForm::checkExternalPlayer);
    connect(ui->filterLengthCheck, &QCheckBox::toggled, this, [this](bool c) { ui->filterLength->setEnabled(c); });
    connect(ui->showFilteredTerms, &QPushButton::clicked, this, &SettingsForm::showTermFilterTable);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &SettingsForm::currentChanged);

    setupSaveButton(ui->saveButton, true);
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
    store.darkTheme = ui->darkTheme->isChecked();
    store.imageCaching = ui->imageCaching->isChecked();
    store.preferLists = ui->preferLists->isChecked();
    // player
    store.externalPlayerPath = ui->externalPlayerEdit->text();
    store.vaapi = ui->vaapi->isChecked();
    // filtering
    store.filterLength = ui->filterLength->value();
    store.filterLengthEnabled = ui->filterLengthCheck->isChecked();

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
        ui->vaapi->setEnabled(false);
    }
}

void SettingsForm::showTermFilterTable()
{
    TermFilterView* fv = new TermFilterView;
    fv->show();
    fv->populateFromSettings();
}
