#include "addplugindialog.h"
#include "ui_addplugindialog.h"
#include "httprequest.h"
#include "qttubeapplication.h"
#include <QFileDialog>
#include <QMessageBox>

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

    try
    {
        QString libraryInput = ui->lineEdit->text();
        if (QFileInfo libraryFileInfo(libraryInput); libraryFileInfo.exists())
        {
            plugin = qtTubeApp->plugins().openPlugin(libraryFileInfo);
        }
        else if (QUrl libraryUrl(libraryInput); libraryUrl.isValid())
        {
            if (libraryUrl.isLocalFile())
            {
                plugin = qtTubeApp->plugins().openPlugin(QFileInfo(libraryUrl.toLocalFile()));
            }
            else if (QString fileName = libraryUrl.fileName(); !fileName.isEmpty())
            {
                QFile temporaryPluginFile(QDir::temp().filePath(libraryUrl.fileName()));
                temporaryPluginFile.open(QFile::WriteOnly);
                temporaryPluginFile.write(blockingRequest(libraryUrl));
                temporaryPluginFile.close();

                plugin = qtTubeApp->plugins().openPlugin(QFileInfo(temporaryPluginFile));
            }
            else
            {
                QMessageBox::critical(this, "Invalid Input", "Input points to an invalid URL.");
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

    QString newPluginPath =
        qtTubeApp->plugins().pluginLoadDirs().front() +
        QDir::separator() +
        plugin.fileInfo.fileName();

    QFileInfo(newPluginPath).dir().mkpath(".");
    QFile::rename(plugin.fileInfo.absoluteFilePath(), newPluginPath);
    plugin.fileInfo.setFile(newPluginPath);

    emit qtTubeApp->activePluginChanged(qtTubeApp->plugins().loadAndInitPlugin(std::move(plugin)));
    done(QDialog::Accepted);
}

QByteArray AddPluginDialog::blockingRequest(const QUrl& url)
{
    HttpReply* reply = HttpRequest().get(url);

    QEventLoop loop;
    connect(reply, &HttpReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    return reply->readAll();
}

void AddPluginDialog::getOpenFile()
{
#if defined(Q_OS_WIN)
    constexpr QLatin1String LibraryFilter("Library files (*.dll)");
#elif defined(Q_OS_MACOS)
    constexpr QLatin1String LibraryFilter("Library files (*.dylib)");
#else
    constexpr QLatin1String LibraryFilter("Library files (*.so)");
#endif

    ui->lineEdit->setText(QFileDialog::getOpenFileName(
        this, "Select a library file...", {}, LibraryFilter));
}
