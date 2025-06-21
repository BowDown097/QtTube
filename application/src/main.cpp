#include "qttubeapplication.h"
#include "mainwindow.h"
#include "ui/forms/livechatwindow.h"

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

    QCommandLineOption channel(QStringList() << "c" << "channel", "View a channel.", "Channel ID", "");
    parser.addOption(channel);

    QCommandLineOption chat("chat", "Open a live chat window.", "Video ID");
    parser.addOption(chat);

    QCommandLineOption version("version", "Displays version information.");
    parser.addOption(version);

    QCommandLineOption video(QStringList() << "v" << "video", "Play a video.", "Video ID", "");
    parser.addOption(video);

    parser.addHelpOption();

    parser.parse(QCoreApplication::arguments());

    if (parser.isSet("help"))
        parser.showHelp();
    if (parser.isSet("version"))
        parser.showVersion();

    if (parser.isSet("chat"))
    {
        qtTubeApp->doInitialSetup();
        if (const PluginData* plugin = qtTubeApp->plugins().activePlugin())
        {
            QtTube::VideoReply* videoReply = plugin->interface->getVideo(parser.value("chat"), {});
            QObject::connect(videoReply, &QtTube::VideoReply::exception, [](const QtTube::PluginException& ex) {
                qDebug() << "Could not open live chat:" << ex.message();
                qtTubeApp->exit(EXIT_FAILURE);
            });
            QObject::connect(videoReply, &QtTube::VideoReply::finished, [](const QtTube::VideoData& data) {
                if (data.initialLiveChatData.has_value())
                {
                    LiveChatWindow liveChatWindow;
                    liveChatWindow.show();
                    liveChatWindow.initialize(data.initialLiveChatData.value(), nullptr);
                }
            });
            return a.exec();
        }
        else
        {
            qDebug() << "No active plugin found.";
            exit(EXIT_FAILURE);
        }
    }

    MainWindow w(parser);
    w.show();

#ifdef QTTUBE_HAS_WAYLAND
    if (a.platformName() == "wayland")
        a.waylandInterface().initialize();
#endif

    return a.exec();
}
