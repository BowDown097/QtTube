#include "qttubeapplication.h"
#include "eastereggs.h"
#include "mainwindow.h"
#include <QMouseEvent>
#include <QPropertyAnimation>

bool QtTubeApplication::notify(QObject* receiver, QEvent* event)
{
    if (receiver->objectName() != "MainWindowWindow")
        return QApplication::notify(receiver, event);

    if (event->type() == QEvent::MouseMove)
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        TopBar* topbar = MainWindow::topbar();
        if (!topbar->alwaysShow)
        {
            if (mouseEvent->pos().y() < topbar->height())
            {
                if (topbar->isHidden() && topbar->animation->state() != QAbstractAnimation::Running)
                {
                    topbar->animation->setStartValue(QRect(0, 0, topbar->width(), 0));
                    topbar->animation->setEndValue(QRect(0, 0, topbar->width(), topbar->height()));
                    disconnect(topbar->animation, &QPropertyAnimation::finished, topbar, nullptr);
                    topbar->animation->start();
                    topbar->show();
                }
            }
            else if (topbar->animation->state() != QAbstractAnimation::Running && !topbar->isHidden())
            {
                int height = topbar->height();
                topbar->animation->setEndValue(QRect(0, 0, topbar->width(), 0));
                topbar->animation->setStartValue(QRect(0, 0, topbar->width(), topbar->height()));
                connect(topbar->animation, &QPropertyAnimation::finished, topbar, [height, topbar] {
                    topbar->hide();
                    topbar->resize(topbar->width(), height);
                });
                topbar->animation->start();
            }
        }
    }
    else if (event->type() == QEvent::KeyPress)
    {
        EasterEggs::checkEasterEggs(static_cast<QKeyEvent*>(event));
    }

    return QApplication::notify(receiver, event);
}
