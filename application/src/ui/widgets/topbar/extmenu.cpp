#include "extmenu.h"

void ExtMenu::mouseReleaseEvent(QMouseEvent* event)
{
    QAction* action = activeAction();
    if (action && action->isEnabled())
    {
        action->setEnabled(false);
        QMenu::mouseReleaseEvent(event);
        action->setEnabled(true);
        hide();
        emit switchActionRequested(action);
    }
    else
    {
        QMenu::mouseReleaseEvent(event);
    }
}
