#include "exttoolbutton.h"
#include <QMenu>
#include <QMouseEvent>

void ExtToolButton::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
        showMenu();
    QToolButton::mouseReleaseEvent(event);
}
