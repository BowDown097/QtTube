#include "continuablelistwidget.h"
#include "innertube.h"
#include <QScrollBar>
#include <QWheelEvent>

ContinuableListWidget::ContinuableListWidget(QWidget* parent) : QListWidget(parent)
{
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &ContinuableListWidget::scrollValueChanged);
}

void ContinuableListWidget::scrollValueChanged(int value)
{
    if (count() > 0 && value >= verticalScrollBar()->maximum() - continuationThreshold &&
        !continuationToken.isEmpty() && !continuationRunning &&
        !InnerTube::instance()->context()->client.visitorData.isEmpty())
    {
        emit continuationReady();
    }
}

// singleStep is reset when changing the list to a grid, this makes sure it stays at 25
// without needing any extra code in individual usages
void ContinuableListWidget::updateGeometries()
{
    QListView::updateGeometries();
    verticalScrollBar()->setSingleStep(25);
}

// circumvent qt bug(?) where QWheelEvent is still accepted when attempting to scroll on a disabled scroll bar.
void ContinuableListWidget::wheelEvent(QWheelEvent* event)
{
    int deltaXAbs = std::abs(event->angleDelta().x());
    int deltaYAbs = std::abs(event->angleDelta().y());
    bool ignoredHorizontalScroll = deltaXAbs > deltaYAbs && !horizontalScrollBar()->isEnabled();
    bool ignoredVerticalScroll = deltaYAbs > deltaXAbs && !verticalScrollBar()->isEnabled();

    if (ignoredHorizontalScroll || ignoredVerticalScroll)
        event->ignore();
    else
        QListWidget::wheelEvent(event);
}
