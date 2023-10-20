#include "tubelabel.h"
#include <QMouseEvent>

TubeLabel::TubeLabel(QWidget* parent, Qt::WindowFlags f) : QLabel(parent, f)
{
    setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
}

TubeLabel::TubeLabel(const InnertubeObjects::InnertubeString& itStr, QWidget* parent, Qt::WindowFlags f) : TubeLabel(parent, f)
{
    setText(itStr.text);
}

TubeLabel::TubeLabel(const QString& text, QWidget* parent, Qt::WindowFlags f) : TubeLabel(parent, f)
{
    setText(text);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void TubeLabel::enterEvent(QEnterEvent*)
#else
void TubeLabel::enterEvent(QEvent*)
#endif
{
    if (clickable)
        setCursor(QCursor(Qt::PointingHandCursor));
    if (underline)
        setStyleSheet("QLabel { text-decoration: underline; }");
}

void TubeLabel::leaveEvent(QEvent*)
{
    if (clickable)
        setCursor(QCursor());
    if (underline)
        setStyleSheet(QString());
}

void TubeLabel::mousePressEvent(QMouseEvent* event)
{
    if (clickable && event->button() == Qt::LeftButton)
        emit clicked();
}
