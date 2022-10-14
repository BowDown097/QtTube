#include "clickablelabel.h"

void ClickableLabel::enterEvent(QEnterEvent*)
{
    setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
    if (underline) setStyleSheet("QLabel { text-decoration: underline; }");
}

void ClickableLabel::leaveEvent(QEvent*)
{
    setCursor(QCursor());
    setStyleSheet("");
}
