#include "fullscreennotification.h"
#include <QGraphicsEffect>
#include <QPainter>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>

namespace QtTubePlugin
{
    constexpr int PadH = 10, PadW = 100;

    FullScreenNotification::FullScreenNotification(QWidget* parent) : QWidget(parent)
    {
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_TransparentForMouseEvents);

        QFont f = font();
        f.setPixelSize(22);
        setFont(f);

        QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect;
        effect->setOpacity(0.6);
        setGraphicsEffect(effect);

        QSequentialAnimationGroup* group = new QSequentialAnimationGroup(this);
        group->addPause(2000);

        QPropertyAnimation* animation = new QPropertyAnimation(effect, "opacity", group);
        animation->setDuration(1200);
        animation->setStartValue(0.6);
        animation->setEndValue(0.0);
        animation->setEasingCurve(QEasingCurve::OutQuad);
        group->addAnimation(animation);

        connect(this, &FullScreenNotification::shown, this, [group] { group->start(); });
        connect(group, &QSequentialAnimationGroup::finished, this, &FullScreenNotification::hide);
    }

    void FullScreenNotification::paintEvent(QPaintEvent* event)
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        const QRect r(QPoint(0, 0), sizeHint());
        const QColor backgroundColor(0, 0, 0);
        const QColor textColor = Qt::white;

        painter.setBrush(backgroundColor);
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(r, 25, 25);

        painter.setPen(textColor);
        painter.drawText(r, Qt::AlignCenter, text());
    }

    void FullScreenNotification::showEvent(QShowEvent* event)
    {
        QWidget::showEvent(event);
        if (!m_previouslyVisible && isVisible())
            emit shown();
        m_previouslyVisible = isVisible();
    }

    QSize FullScreenNotification::sizeHint() const
    {
        const QFontMetrics fm = fontMetrics();
        return QSize(fm.horizontalAdvance(text()) + 2 * PadW, fm.height() + 2 * PadH);
    }

    QString FullScreenNotification::text() const
    {
        return tr("Press Esc to exit full screen mode.");
    }
}
