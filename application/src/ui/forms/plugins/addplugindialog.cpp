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
                    ? QDir::temp().filePath(QUuid::createUuid().toString(QUuid::Id128) + libraryExtension)
                    : QDir::temp().filePath(libraryUrl.fileName()));
                if (!temporaryPluginFile.open(QFile::WriteOnly))
                {
                    QMessageBox::critical(this, QString(), "Could not open plugin file for writing.");
                    done(QDialog::Rejected);
                    return;
                }

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
                done(QDialog::Rejected);
                return;
            }
        }
    }
    catch (const PluginLoadException& ex)
    {
        QMessageBox::critical(this, "Plugin Error", ex.message());
        done(QDialog::Rejected);
        return;
    }

    QFileInfo(pluginPath).dir().mkpath(".");

    if (!QFile::rename(plugin.fileInfo.absoluteFilePath(), pluginPath))
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
