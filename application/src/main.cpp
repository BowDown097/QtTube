#include "qttubeapplication.h"
#include "mainwindow.h"
#include "ui/forms/livechatwindow.h"
#include <QCommandLineParser>

int showChat(QtTubeApplication& a, QCommandLineParser& parser)
{
    a.doInitialSetup();

    PluginData* plugin = parser.isSet("use-plugin")
        ? a.plugins().findPlugin(parser.value("use-plugin"))
        : a.plugins().activePlugin();
    if (!plugin)
    {
        qCritical() << "Could not open live chat: Plugin not found.";
        return EXIT_FAILURE;
    }

    QtTubePlugin::VideoReply* videoReply = plugin->interface->getVideo(parser.value("chat"));
    if (!videoReply)
    {
        qCritical() << "Could not open live chat: No method has been provided.";
        return EXIT_FAILURE;
    }

    QObject::connect(videoReply, &QtTubePlugin::VideoReply::exception, [&a](const QtTubePlugin::Exception& ex) {
        qCritical() << "Could not open live chat:" << ex.message();
        a.exit(EXIT_FAILURE);
    });

    QObject::connect(videoReply, &QtTubePlugin::VideoReply::finished, [plugin](const QtTubePlugin::VideoData& data) {
        if (data.initialLiveChatData.has_value())
        {
            LiveChatWindow* window = new LiveChatWindow(plugin);
            window->show();
            window->initialize(data.initialLiveChatData.value(), nullptr);
        }
    });

    return a.exec();
}

int main(int argc, char *argv[])
{
    QApplication::setApplicationName(QTTUBE_APP_NAME);
    QApplication::setApplicationVersion(QTTUBE_VERSION_NAME);

#if QT_VERSION <= QT_VERSION_CHECK(6, 0, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

#ifdef Q_OS_WIN
    QApplication::setStyle("fusion");
    qputenv("QT_QPA_PLATFORM", "windows:darkmode=2");
#endif

    QtTubeApplication a(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(QTTUBE_APP_DESC);
    parser.addHelpOption();
    parser.addOption(QCommandLineOption({"c",  "channel"}, "View a channel.", "channel ID"));
    parser.addOption(QCommandLineOption("chat", "Open a live chat window.", "video ID"));
    parser.addOption(QCommandLineOption("use-plugin", "The plugin to use for this session.", "plugin name"));
    parser.addOption(QCommandLineOption("version", "Displays version information."));
    parser.addOption(QCommandLineOption({"v", "video"}, "Play a video.", "video ID"));
    parser.parse(QCoreApplication::arguments());

    // immediately returning options
    if (parser.isSet("chat"))
        return showChat(a, parser);
    if (parser.isSet("help"))
        parser.showHelp();
    if (parser.isSet("version"))
        parser.showVersion();

    MainWindow w(parser);
    w.show();

#ifdef QTTUBE_HAS_WAYLAND
    if (a.platformName() == "wayland")
        a.waylandInterface().initialize();
#endif

    return a.exec();
}
