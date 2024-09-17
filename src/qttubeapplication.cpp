#include "qttubeapplication.h"
#include "eastereggs.h"
#include "mainwindow.h"

bool QtTubeApplication::notify(QObject* receiver, QEvent* event)
{
    if (receiver->objectName() == "MainWindowWindow")
    {
        switch (event->type())
        {
        case QEvent::KeyPress:
            EasterEggs::checkEasterEggs(static_cast<QKeyEvent*>(event));
            break;
        case QEvent::MouseMove:
            if (settings().autoHideTopBar)
                MainWindow::topbar()->handleMouseEvent(static_cast<QMouseEvent*>(event));
            break;
        default: break;
        }
    }

    return QApplication::notify(receiver, event);
}
