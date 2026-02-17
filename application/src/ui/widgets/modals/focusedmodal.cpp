#include "focusedmodal.h"
#include <QGraphicsOpacityEffect>
#include <QKeyEvent>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>

FocusedModal::FocusedModal(QWidget* parent)
    : QWidget(parent), m_backdrop(new QWidget(parent))
{
    if (!parent)
        throw std::runtime_error("FocusedModal created with null parent");

    QPalette backdropPalette;
    backdropPalette.setColor(QPalette::Window, QColor(0, 0, 0, 180));

    m_backdrop->setAutoFillBackground(true);
    m_backdrop->setPalette(backdropPalette);
    m_backdrop->raise();

    setAttribute(Qt::WA_DeleteOnClose);
    setAutoFillBackground(true);
    raise();

    setFocusPolicy(Qt::StrongFocus);
    setFocus(Qt::PopupFocusReason);

    QPropertyAnimation* thisAnim = animationFor(this);
    QPropertyAnimation* backdropAnim = animationFor(m_backdrop.get());

    QParallelAnimationGroup* animGroup = new QParallelAnimationGroup;
    animGroup->addAnimation(thisAnim);
    animGroup->addAnimation(backdropAnim);
    animGroup->start(QAbstractAnimation::DeleteWhenStopped);

    m_backdrop->show();
    show();

    parent->installEventFilter(this);
}

QPropertyAnimation* FocusedModal::animationFor(QWidget* widget)
{
    QGraphicsOpacityEffect* eff = new QGraphicsOpacityEffect(widget);
    widget->setGraphicsEffect(eff);

    QPropertyAnimation* anim = new QPropertyAnimation(eff, "opacity");
    anim->setDuration(200);
    anim->setStartValue(0);
    anim->setEndValue(1);
    anim->setEasingCurve(QEasingCurve::Linear);

    return anim;
}

bool FocusedModal::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::Resize)
        reorient();
    return false;
}

void FocusedModal::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
        close();
    else
        QWidget::keyPressEvent(event);
}

void FocusedModal::reorient()
{
    m_backdrop->setGeometry(parentWidget()->rect());
    move((m_backdrop->width() - width()) / 2,
         (m_backdrop->height() - height()) / 2);
}
