#include "tubelabel.h"
#include <QMouseEvent>

TubeLabel::TubeLabel(QWidget* parent, Qt::WindowFlags f) : QLabel(parent, f)
{
    setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
}

TubeLabel::TubeLabel(const QString& text, QWidget* parent, Qt::WindowFlags f) : TubeLabel(parent, f)
{
    setText(text);
}

void TubeLabel::enterEvent(QEnterEvent*)
{
    if (clickable)
        setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
    if (underline)
        setStyleSheet("QLabel { text-decoration: underline; }");
}

void TubeLabel::leaveEvent(QEvent*)
{
    if (clickable)
        setCursor(QCursor());
    if (underline)
        setStyleSheet("");
}

void TubeLabel::mousePressEvent(QMouseEvent* event)
{
    if (clickable && event->button() == Qt::LeftButton)
        emit clicked();
}
