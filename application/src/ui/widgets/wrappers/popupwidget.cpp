#include "popupwidget.h"
#include <QLayout>

PopupWidget::PopupWidget(QWidget* base, QWidget* anchor)
    : QFrame(nullptr, Qt::Popup | Qt::FramelessWindowHint),
      m_anchor(anchor),
      m_base(base)
{
    connect(base->parent(), &QObject::destroyed, this, &PopupWidget::deleteLater);
    base->setParent(this);
}

QRect PopupWidget::baseGeometry() const
{
    if (!m_anchor || !m_base)
        return {};
    return QRect(m_anchor->mapToGlobal(QPoint(0, 0)), m_base->size());
}

QRect PopupWidget::desiredOpenGeometry() const
{
    QRect geo = baseGeometry();

    QSize targetSize = testAttribute(Qt::WA_WState_Created) ? size() : geo.size();
    if (targetSize.isEmpty())
        targetSize = m_base->sizeHint();

    return !geo.isNull()
        ? QRect(QPoint(geo.left(), geo.top() - targetSize.height()), targetSize)
        : QRect(pos(), targetSize);
}

void PopupWidget::showPopup()
{
    // we need an activated layout to know our target size
    if (layout() && !testAttribute(Qt::WA_WState_Created))
        layout()->activate();

    setGeometry(desiredOpenGeometry());
    show();
}
