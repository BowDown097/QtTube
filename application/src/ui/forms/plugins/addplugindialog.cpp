#include "addplugindialog.h"
#include "ui_addplugindialog.h"
#include "qttube-plugin/utils/httprequest.h"
#include "qttubeapplication.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QUuid>

namespace
{
#if defined(Q_OS_WIN)
const QString libraryExtension = QStringLiteral(".dll");
#elif defined(Q_OS_MACOS)
const QString libraryExtension = QStringLiteral(".dylib");
#else
const QString libraryExtension = QStringLiteral(".so");
#endif
}

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
    const QDir& pluginsDir = qtTubeApp->plugins().pluginLoadDirs().front();
    PluginData plugin;
    QString pluginPath;

    try
    {
        PluginSource source = resolvePluginSource(ui->lineEdit->text());
        plugin = qtTubeApp->plugins().openPlugin(source.fileInfo);
        pluginPath = pluginsDir.filePath(source.targetFileName);
    }
    catch (const PluginLoadException& ex)
    {
        QMessageBox::critical(this, "Plugin Error", ex.message());
        done(QDialog::Rejected);
        return;
    }

    pluginsDir.mkpath(".");

    if (!QFile::rename(plugin.fileInfo.filePath(), pluginPath))
    {
        QMessageBox::critical(this, QString(), "Could not relocate plugin file to plugin folder.");
        done(QDialog::Rejected);
        return;
    }

    plugin.fileInfo.setFile(pluginPath);

    PluginData* loadedPlugin = qtTubeApp->plugins().loadAndInitPlugin(std::move(plugin));
    if (!qtTubeApp->plugins().activePlugin() ||
        QMessageBox::question(this, QString(), "Make this plugin the active plugin?") == QMessageBox::Yes)
    {
        loadedPlugin->active = true;
        emit qtTubeApp->activePluginChanged(loadedPlugin);
    }

    done(QDialog::Accepted);
}

void AddPluginDialog::getOpenFile()
{
    ui->lineEdit->setText(QFileDialog::getOpenFileName(
        this, "Select a library file...", {}, "Library files (*" + libraryExtension + ')'));
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

    bool fullyTemp = !QLibrary::isLibrary(fileName);
    QFile tempFile(fullyTemp
        ? QDir::temp().filePath(QUuid::createUuid().toString(QUuid::Id128) + libraryExtension)
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

AddPluginDialogEntry::AddPluginDialogEntry(PluginData* data, QWidget* parent)
    : BasePluginEntry(parent),
      m_activeButton(new QRadioButton(this)),
      m_data(data)
{
    m_activeButton->setChecked(data->active);
    m_topLayout->insertWidget(0, m_activeButton);

    if (data->settings->window())
    {
        QPushButton* openSettingsButton = new QPushButton("Open Settings", this);
        m_buttonsLayout->addWidget(openSettingsButton);
        connect(openSettingsButton, &QPushButton::clicked, this, [data] {
            QWidget* window = data->settings->window();
            window->setAttribute(Qt::WA_DeleteOnClose);
            window->show();
        });
    }

    m_buttonsLayout->addStretch();
}
