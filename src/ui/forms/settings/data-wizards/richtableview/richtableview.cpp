#include "richtableview.h"
#include "richitemdelegate.h"
#include <QDesktopServices>
#include <QMouseEvent>

QString RichTableView::anchorAt(const QPoint& point) const
{
    QModelIndex index = indexAt(point);
    if (!index.isValid())
        return QString();

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const RichItemDelegate* delegate = qobject_cast<RichItemDelegate*>(itemDelegateForIndex(index));
#else
    const RichItemDelegate* delegate = qobject_cast<RichItemDelegate*>(itemDelegate(index));
#endif

    if (!delegate)
        return QString();

    QPoint relativeClickPosition = point - visualRect(index).topLeft();
    QString html = model()->data(index, Qt::DisplayRole).toString();
    return delegate->anchorAt(html, relativeClickPosition);
}

void RichTableView::mouseMoveEvent(QMouseEvent* event)
{
    QString anchor = anchorAt(event->pos());
    if (anchor != mousePressAnchor)
        mousePressAnchor.clear();

    if (anchor == lastHoveredAnchor)
        return;

    lastHoveredAnchor = anchor;
    setCursor(!lastHoveredAnchor.isEmpty() ? QCursor(Qt::PointingHandCursor) : QCursor());
}

void RichTableView::mousePressEvent(QMouseEvent* event)
{
    mousePressAnchor = anchorAt(event->pos());
    QTableView::mousePressEvent(event);
}

void RichTableView::mouseReleaseEvent(QMouseEvent* event)
{
    if (!mousePressAnchor.isEmpty())
    {
        if (anchorAt(event->pos()) == mousePressAnchor)
            QDesktopServices::openUrl(QUrl(mousePressAnchor));
        mousePressAnchor.clear();
    }

    QTableView::mouseReleaseEvent(event);
}
