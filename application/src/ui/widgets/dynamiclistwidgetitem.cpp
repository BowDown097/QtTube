#include "dynamiclistwidgetitem.h"
#include <QResizeEvent>

bool ResizeEventFilter::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::Resize)
        emit resizing();
    return QObject::eventFilter(obj, event);
}

void DynamicListWidgetItem::setWidget(QWidget* widget)
{
    setSizeHint(widget->sizeHint());
    listWidget()->addItem(this);
    listWidget()->setItemWidget(this, widget);

    ResizeEventFilter* eventFilter = new ResizeEventFilter(widget);
    widget->installEventFilter(eventFilter);

    QObject::connect(eventFilter, &ResizeEventFilter::resizing, [this, widget] { setSizeHint(widget->size()); });
}
