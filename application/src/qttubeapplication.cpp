#include "qttubeapplication.h"
#include "mainwindow.h"
#include "ui/views/viewcontroller.h"
#include "ui/widgets/topbar/topbar.h"
#include "utils/uiutils.h"
#include <QDesktopServices>
#include <QMessageBox>
#include <QStyle>

#ifdef Q_OS_WIN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

QtTubeApplication::QtTubeApplication(int& argc, char** argv)
    : QApplication(argc, argv)
{
    m_commandLineParser.setApplicationDescription(QTTUBE_APP_DESC);
    m_commandLineParser.addHelpOption();
    m_commandLineParser.addOption(QCommandLineOption({"c",  "channel"}, "View a channel.", "channel ID"));
    m_commandLineParser.addOption(QCommandLineOption("chat", "Open a live chat window.", "video ID"));
    m_commandLineParser.addOption(QCommandLineOption("use-plugin", "The plugin to use for this session.", "plugin name"));
    m_commandLineParser.addOption(QCommandLineOption("version", "Displays version information."));
    m_commandLineParser.addOption(QCommandLineOption({"v", "video"}, "Play a video.", "video ID"));
    m_commandLineParser.parse(arguments());
}

void QtTubeApplication::doInitialSetup()
{
    // on windows, rpath is not supported for plugins, so we'll add lib directories manually
#ifdef Q_OS_WIN
    SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    for (const QDir& dir : qtTubeApp->plugins().libraryLoadDirs())
        AddDllDirectory(qUtf16Printable(QDir::toNativeSeparators(dir.path())));
#endif

    m_settings.init();
    m_plugins.reloadPlugins();

    UIUtils::g_defaultStyle = style()->objectName();
    UIUtils::setAppStyle(m_settings.appStyle, m_settings.darkTheme);
}

void QtTubeApplication::handleUrlOrID(const QString& in)
{
    if (PluginEntry* plugin = qtTubeApp->plugins().activePlugin())
    {
        if (QtTubePlugin::ResolveUrlReply* reply = plugin->interface->resolveUrlOrID(in))
        {
            connect(reply, &QtTubePlugin::ResolveUrlReply::exception, this, [this](const QtTubePlugin::Exception& ex) {
                QMessageBox::critical(nullptr, "Error Resolving Input", ex.message());
            });
            connect(reply, &QtTubePlugin::ResolveUrlReply::finished, this, [this, plugin](const QtTubePlugin::ResolveUrlData& data) {
                switch (data.target)
                {
                case QtTubePlugin::ResolveUrlTarget::Channel:
                    ViewController::loadChannel(data.data, plugin);
                    break;
                case QtTubePlugin::ResolveUrlTarget::Search:
                    emit UIUtils::getMainWindow()->topbar()->searchBox->searchRequested(data.data, SearchBox::SearchType::ByQuery);
                    break;
                case QtTubePlugin::ResolveUrlTarget::Video:
                    ViewController::loadVideo(data.data, plugin, data.videoProgress, nullptr, data.continuePlayback);
                    break;
                case QtTubePlugin::ResolveUrlTarget::PlainUrl:
                    QDesktopServices::openUrl(data.data);
                    break;
                case QtTubePlugin::ResolveUrlTarget::NotResolved:
                    QMessageBox::warning(nullptr, "No Results Found", "Nothing could be found based on your input.");
                    break;
                }
            });
        }
        else
        {
            QMessageBox::warning(nullptr, "Feature Not Available", "This feature is not supported by the active plugin.");
        }
    }
}

bool QtTubeApplication::isPortableBuild()
{
    static const bool result = QtTubePlugin::isPortableBuild();
    return result;
}

bool QtTubeApplication::isSelfContainedBuild()
{
    static const bool result = isPortableBuild() || QtTubePlugin::isSelfContainedBuild();
    return result;
}

bool QtTubeApplication::notify(QObject* receiver, QEvent* event)
{
    if (m_settings.autoHideTopBar && event->type() == QEvent::MouseMove && receiver->objectName() == "MainWindowWindow")
        UIUtils::getMainWindow()->topbar()->handleMouseEvent(static_cast<QMouseEvent*>(event));
    return QApplication::notify(receiver, event);
}
