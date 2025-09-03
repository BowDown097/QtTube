#include "qttubeapplication.h"
#include "mainwindow.h"
#include "ui/views/viewcontroller.h"
#include "utils/uiutils.h"
#include <QDesktopServices>
#include <QMessageBox>

#ifdef Q_OS_WIN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

void QtTubeApplication::doInitialSetup()
{
    // on windows, rpath is not supported for plugins. add lib directories manually
#ifdef Q_OS_WIN
    SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    const QStringList libraryLoadDirs = {
        QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + QDir::separator() + "plugin-libs",
        qApp->applicationDirPath() + QDir::separator() + "plugin-libs"
    };

    for (const QString& dir : libraryLoadDirs)
        AddDllDirectory(qUtf16Printable(QDir::toNativeSeparators(dir)));
#endif

    m_plugins.reloadPlugins();
    m_settings.initialize();

    UIUtils::g_defaultStyle = style()->objectName();
    UIUtils::setAppStyle(m_settings.appStyle, m_settings.darkTheme);
}

void QtTubeApplication::handleUrlOrID(const QString& in)
{
    if (PluginData* plugin = qtTubeApp->plugins().activePlugin())
    {
        if (QtTubePlugin::ResolveUrlReply* reply = plugin->interface->resolveUrlOrID(in))
        {
            connect(reply, &QtTubePlugin::ResolveUrlReply::exception, this, [this](const QtTubePlugin::Exception& ex) {
                QMessageBox::critical(nullptr, "Error in resolution", ex.message());
            });
            connect(reply, &QtTubePlugin::ResolveUrlReply::finished, this, [this](const QtTubePlugin::ResolveUrlData& data) {
                switch (data.target)
                {
                case QtTubePlugin::ResolveUrlTarget::Channel:
                    ViewController::loadChannel(data.data);
                    break;
                case QtTubePlugin::ResolveUrlTarget::Search:
                    emit MainWindow::topbar()->searchBox->searchRequested(data.data, SearchBox::SearchType::ByQuery);
                    break;
                case QtTubePlugin::ResolveUrlTarget::Video:
                    ViewController::loadVideo(data.data, data.videoProgress, nullptr, data.continuePlayback);
                    break;
                case QtTubePlugin::ResolveUrlTarget::PlainUrl:
                    QDesktopServices::openUrl(data.data);
                    break;
                case QtTubePlugin::ResolveUrlTarget::NotResolved:
                    QMessageBox::warning(nullptr, "Nothing found!", "Could not find anything from your input.");
                    break;
                }
            });
        }
        else
        {
            QMessageBox::warning(nullptr, "Failed to perform resolution", "No method has been provided.");
        }
    }
}

bool QtTubeApplication::notify(QObject* receiver, QEvent* event)
{
    if (m_settings.autoHideTopBar && event->type() == QEvent::MouseMove && receiver->objectName() == "MainWindowWindow")
        MainWindow::topbar()->handleMouseEvent(static_cast<QMouseEvent*>(event));
    return QApplication::notify(receiver, event);
}
