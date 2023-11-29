#include "qttubeapplication.h"
#include "eastereggs.h"
#include "mainwindow.h"

bool QtTubeApplication::notify(QObject* receiver, QEvent* event)
{
    if (receiver->objectName() == "MainWindowWindow")
    {
        if (event->type() == QEvent::MouseMove)
            MainWindow::topbar()->handleMouseEvent(static_cast<QMouseEvent*>(event));
        else if (event->type() == QEvent::KeyPress)
            EasterEggs::checkEasterEggs(static_cast<QKeyEvent*>(event));
    }

    return QApplication::notify(receiver, event);
}
