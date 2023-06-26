#include "qiconwidget.h"
#include "ui/uiutilities.h"
#include <QMouseEvent>

QIconWidget::QIconWidget(const QString& iconId, const QSize& size, QWidget* parent)
    : QSvgWidget(parent)
{
    setFixedSize(size);
    load(UIUtilities::resolveThemedIconName(iconId));
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void QIconWidget::enterEvent(QEnterEvent*)
#else
void QIconWidget::enterEvent(QEvent*)
#endif
{
    if (clickable)
        setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
}

void QIconWidget::leaveEvent(QEvent*)
{
    if (clickable)
        setCursor(QCursor());
}

void QIconWidget::mousePressEvent(QMouseEvent* event)
{
    if (clickable && event->button() == Qt::LeftButton)
        emit clicked();
}

void QIconWidget::setIcon(const QString& iconId, const QPalette& pal)
{
    load(UIUtilities::resolveThemedIconName(iconId, pal));
}
