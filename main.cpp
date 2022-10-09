#include "mainwindow.h"
#include "qttubeapplication.hpp"

int main(int argc, char *argv[])
{
    QtTubeApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
