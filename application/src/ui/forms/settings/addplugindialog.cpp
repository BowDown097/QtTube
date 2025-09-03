#include "addplugindialog.h"
#include "ui_addplugindialog.h"
#include "httprequest.h"
#include "qttubeapplication.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QUuid>

#if defined(Q_OS_WIN)
constexpr QLatin1String LibraryExtension(".dll");
#elif defined(Q_OS_MACOS)
constexpr QLatin1String LibraryExtension(".dylib");
#else
constexpr QLatin1String LibraryExtension(".so");
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
    PluginData plugin;
    QString pluginPath = qtTubeApp->plugins().pluginLoadDirs().front() + QDir::separator();

    try
    {
        QString libraryInput = ui->lineEdit->text();
        if (QFileInfo libraryFileInfo(libraryInput); libraryFileInfo.exists())
        {
            plugin = qtTubeApp->plugins().openPlugin(libraryFileInfo);
            pluginPath += libraryFileInfo.fileName();
        }
        else if (QUrl libraryUrl(libraryInput); libraryUrl.isValid())
        {
            if (libraryUrl.isLocalFile())
            {
                QFileInfo localFileInfo(libraryUrl.toLocalFile());
                plugin = qtTubeApp->plugins().openPlugin(localFileInfo);
                pluginPath += localFileInfo.fileName();
            }
            else if (QString fileName = libraryUrl.fileName(); !fileName.isEmpty())
            {
                bool fullyTemp = !QLibrary::isLibrary(fileName);
                QFile temporaryPluginFile(fullyTemp
                    ? QDir::temp().filePath(QUuid::createUuid().toString(QUuid::Id128) + LibraryExtension)
                    : QDir::temp().filePath(libraryUrl.fileName()));
                temporaryPluginFile.open(QFile::WriteOnly);

                HttpReply* reply = HttpRequest().writingToIODevice(&temporaryPluginFile).get(libraryUrl);
                QEventLoop loop;
                connect(reply, &HttpReply::finished, &loop, &QEventLoop::quit);
                loop.exec();

                temporaryPluginFile.close();

                plugin = qtTubeApp->plugins().openPlugin(QFileInfo(temporaryPluginFile));

                QString dispFileName = reply->getFileName();
                pluginPath += (fullyTemp && !dispFileName.isEmpty()) ? dispFileName : fileName;
            }
            else
            {
                QMessageBox::critical(this, "Invalid Input", "The provided input is not a valid URL.");
            }
        }
    }
    catch (const PluginLoadException& ex)
    {
        QMessageBox::critical(this, "Plugin Error", ex.message());
        done(QDialog::Rejected);
        return;
    }

    if (!qtTubeApp->plugins().activePlugin())
        plugin.active = true;

    QFileInfo(pluginPath).dir().mkpath(".");
    QFile::rename(plugin.fileInfo.absoluteFilePath(), pluginPath);
    plugin.fileInfo.setFile(pluginPath);

    emit qtTubeApp->activePluginChanged(qtTubeApp->plugins().loadAndInitPlugin(std::move(plugin)));
    done(QDialog::Accepted);
}

void AddPluginDialog::getOpenFile()
{
    ui->lineEdit->setText(QFileDialog::getOpenFileName(
        this, "Select a library file...", {}, "Library files (*" + LibraryExtension + ')'));
}
