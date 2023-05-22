#include "qttubeapplication.h"
#include "ui/forms/mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication::setApplicationName(QTTUBE_APP_NAME);
    QApplication::setApplicationVersion(QTTUBE_VERSION);
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
    QStringList unknownNames = parser.unknownOptionNames();
    if (unknownNames.size() > 0) parser.showHelp(1);
    if (parser.isSet("help")) parser.showHelp();
    if (parser.isSet("version"))
    {
#if QT_VERSION >= 0x050400
        parser.showVersion();
#else
        QTextStream out(stdout);
        out << qPrintable(QTTUBE_APP_NAME) << " " << qPrintable(QTTUBE_VERSION) << std::endl;
        return 0;
#endif
    }

    MainWindow w(parser);
    w.show();
    return a.exec();
}
