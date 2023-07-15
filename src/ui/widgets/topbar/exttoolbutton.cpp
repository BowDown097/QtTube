#include "exttoolbutton.h"
#include <QMenu>
#include <QMouseEvent>

void ExtToolButton::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
        showMenu();
    QToolButton::mousePressEvent(event);
}
