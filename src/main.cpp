#include "qttubeapplication.h"
#include "mainwindow.h"

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

    MainWindow w(parser);
    w.show();

// see comment in OSUtils::suspendIdleSleep for why this is commented out.
/*
#ifdef QTTUBE_HAS_WAYLAND
    if (a.platformName() == "wayland")
        a.waylandInterface().initialize(w.windowHandle());
#endif
*/

    return a.exec();
}
