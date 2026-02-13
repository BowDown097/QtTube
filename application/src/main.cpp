#include "qttubeapplication.h"
#include "mainwindow.h"
#include "ui/forms/livechatwindow.h"
#include <QCommandLineParser>

int showChat(QtTubeApplication& a, QCommandLineParser& parser)
{
    a.doInitialSetup();

    PluginData* plugin = a.plugins().activePlugin();
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
    QCommandLineParser& parser = a.commandLineParser();

    // these options all immediately return
    if (parser.isSet("chat"))
        return showChat(a, parser);
    if (parser.isSet("help"))
        parser.showHelp();
    if (parser.isSet("version"))
        parser.showVersion();

    MainWindow w;
    w.show();

#ifdef QTTUBE_HAS_WAYLAND
    if (a.platformName() == "wayland")
        a.waylandInterface().initialize();
#endif

    return a.exec();
}
