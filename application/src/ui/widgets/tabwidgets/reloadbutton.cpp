#include "reloadbutton.hpp"
#include "utils/uiutils.hpp"
#include <QStyleOption>
#include <QStylePainter>

ReloadButton::ReloadButton(QWidget* parent)
    : QPushButton(parent),
      m_movie(new QMovie(UIUtils::resolveThemedIconName("spinner", "gif"), QByteArray(), this))
{
    m_movie->jumpToFrame(0);
    connect(m_movie, &QMovie::frameChanged, this, qOverload<>(&ReloadButton::update));
}

void ReloadButton::paintEvent(QPaintEvent* event)
{
    QStylePainter painter(this);

    QStyleOptionTab tabOption;
    tabOption.initFrom(this);
    tabOption.state &= ~(QStyle::State_HasFocus | QStyle::State_MouseOver);
    tabOption.rect = rect().adjusted(0, 0, 0, -1);

    if (isDown())
        tabOption.state |= QStyle::State_Sunken;
    if (hasFocus())
        tabOption.state |= QStyle::State_HasFocus;
    if (!isEnabled())
        tabOption.state &= ~QStyle::State_Enabled;
    if (isActiveWindow())
        tabOption.state |= QStyle::State_Active;

    tabOption.state.setFlag(QStyle::State_MouseOver, underMouse());
    tabOption.shape = QTabBar::RoundedNorth;

    painter.drawControl(QStyle::CE_TabBarTabShape, tabOption);

    const QRect cr = contentsRect();
    const qreal dpr = devicePixelRatio();

    QPixmap frame = m_movie->currentPixmap().scaled(
        cr.size() * dpr, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    frame.setDevicePixelRatio(dpr);

    painter.drawPixmap(
        cr.topLeft() + QPoint(
            (cr.width() - frame.width() / dpr) / 2,
            (cr.height() - frame.height() / dpr) / 2),
        frame);
}

QSize ReloadButton::sizeHint() const
{
    return size();
}