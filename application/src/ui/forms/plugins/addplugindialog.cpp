#include "addplugindialog.hpp"
#include "ui_addplugindialog.h"
#include "qttubeapplication.hpp"
#include <QFileDialog>
#include <QMessageBox>
#include <qttube-plugin/components/settings/settingsstore.h>
#include <qttube-plugin/utils/httprequest.h>
#include <QUuid>

#if defined(Q_OS_WIN)
constexpr QLatin1String libraryExtension(".dll");
#elif defined(Q_OS_MACOS)
constexpr QLatin1String libraryExtension(".dylib");
#else
constexpr QLatin1String libraryExtension(".so");
#endif

AddPluginDialog::AddPluginDialog(QWidget* parent)
    : QDialog(parent), ui(new Ui::AddPluginDialog)
{
    ui->setupUi(this);
    connect(ui->addPluginButton, &QPushButton::clicked, this, &AddPluginDialog::attemptAdd);
    connect(ui->openButton, &QPushButton::clicked, this, &AddPluginDialog::getOpenFile);
}

AddPluginDialog::~AddPluginDialog()
{
    delete ui;
}

void AddPluginDialog::attemptAdd()
{
    try
    {
        const QDir& pluginsDir = qtTubeApp->plugins().pluginLoadDirs().front();
        pluginsDir.mkpath(".");

        PluginSource source = resolvePluginSource(ui->lineEdit->text());
        QString targetPluginPath = pluginsDir.filePath(source.targetFileName);

        if (!QFile::rename(source.fileInfo.filePath(), targetPluginPath))
        {
            QMessageBox::critical(this, QString(), "Could not relocate plugin file to plugin folder.");
            done(QDialog::Rejected);
            return;
        }

        source.fileInfo.setFile(targetPluginPath);

        PluginEntry* plugin = qtTubeApp->plugins().registerPlugin(std::move(source.fileInfo));
        if (!qtTubeApp->plugins().activePlugin() ||
            QMessageBox::question(this, QString(), "Make this plugin the active plugin?") == QMessageBox::Yes)
        {
            plugin->active = true;
            emit qtTubeApp->activePluginChanged(plugin);
        }

        done(QDialog::Accepted);
    }
    catch (const PluginLoadException& ex)
    {
        QMessageBox::critical(this, "Plugin Error", ex.message());
        done(QDialog::Rejected);
    }
}

void AddPluginDialog::getOpenFile()
{
    ui->lineEdit->setText(QFileDialog::getOpenFileName(
        this, "Select a plugin file...", {}, "Plugin files (*" % libraryExtension % " *.js)"));
}

AddPluginDialog::PluginSource AddPluginDialog::resolvePluginSource(const QString& input)
{
    // case 1: direct file path
    if (QFileInfo fileInfo(input); fileInfo.exists())
    {
        return {
            .fileInfo = fileInfo,
            .targetFileName = fileInfo.fileName()
        };
    }

    QUrl url(input);
    if (!url.isValid())
        throw PluginLoadException("The provided input is not a valid file or URL.");

    // case 2: local file URL
    if (url.isLocalFile())
    {
        QFileInfo fileInfo(url.toLocalFile());
        if (!fileInfo.exists())
            throw PluginLoadException("The provided URL points to a nonexistent local file.");

        return {
            .fileInfo = fileInfo,
            .targetFileName = fileInfo.fileName()
        };
    }

    // case 3: remote URL
    QString fileName = url.fileName();
    if (fileName.isEmpty())
        throw PluginLoadException("The provided URL does not point to a local or remote file.");

    bool fullyTemp = !PluginEntry::isPluginFile(fileName);
    QFile tempFile(fullyTemp
        ? QDir::temp().filePath(QUuid::createUuid().toString(QUuid::Id128))
        : QDir::temp().filePath(fileName));

    if (!tempFile.open(QFile::WriteOnly))
        throw PluginLoadException("Could not open plugin file for writing.");

    HttpReply* reply = HttpRequest().writingToIODevice(&tempFile).get(url);

    QEventLoop loop;
    connect(reply, &HttpReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    tempFile.close();
    QString displayFileName = reply->getFileName();

    return {
        .fileInfo = QFileInfo(tempFile),
        .targetFileName = (fullyTemp && !displayFileName.isEmpty()) ? displayFileName : fileName
    };
}

AddPluginDialogEntry::AddPluginDialogEntry(PluginEntry* plugin, QWidget* parent)
    : BasePluginEntry(parent),
      m_activeButton(new QRadioButton(this)),
      m_plugin(plugin)
{
    m_activeButton->setChecked(plugin->active);
    m_topLayout->insertWidget(0, m_activeButton);

    if (plugin->settingsStore)
    {
        QPushButton* openSettingsButton = new QPushButton("Open Settings", this);
        m_buttonsLayout->addWidget(openSettingsButton);
        connect(openSettingsButton, &QPushButton::clicked, this, [=] {
            QWidget* window = plugin->settingsStore->window();
            window->setAttribute(Qt::WA_DeleteOnClose);
            window->show();
        });
    }

    m_buttonsLayout->addStretch();
}
