#include "fullscreennotification.h"
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>

FullScreenNotification::FullScreenNotification(QWidget* parent) : QLabel(parent), m_previouslyVisible(false)
{
    setText(tr("Press Esc to exit full screen mode."));
    setStyleSheet(R"(
        font-size: 22px;
        color: white;
        background-color: black;
        padding: 10px 100px;
        border-radius: 25px;
    )");
    setAttribute(Qt::WA_TransparentForMouseEvents);

    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect;
    effect->setOpacity(0.6);
    setGraphicsEffect(effect);

    QSequentialAnimationGroup* animations = new QSequentialAnimationGroup(this);
    animations->addPause(2000);
    QPropertyAnimation* opacityAnimation = new QPropertyAnimation(effect, "opacity", animations);
    opacityAnimation->setDuration(1200);
    opacityAnimation->setStartValue(0.6);
    opacityAnimation->setEndValue(0.0);
    opacityAnimation->setEasingCurve(QEasingCurve::OutQuad);
    animations->addAnimation(opacityAnimation);

    connect(this, &FullScreenNotification::shown, this, [animations](){ animations->start(); });
    connect(animations, &QAbstractAnimation::finished, this, [this](){ this->hide(); });
}

void FullScreenNotification::showEvent(QShowEvent *event)
{
    QLabel::showEvent(event);
    if (!m_previouslyVisible && isVisible())
        emit shown();
    m_previouslyVisible = isVisible();
}
