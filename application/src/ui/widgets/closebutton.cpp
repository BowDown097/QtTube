#include "closebutton.h"
#include <QAction>

CloseButton::CloseButton(QWidget* parent) : QToolButton(parent), m_action(new QAction(this))
{
    m_action->setText("X");
    setDefaultAction(m_action);
    setToolTip(QString());
}
