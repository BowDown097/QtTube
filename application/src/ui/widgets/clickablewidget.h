#pragma once
#include <wobjectimpl.h>
#include <QMouseEvent>
#include <QWidget>

template<std::derived_from<QWidget> W = QWidget>
class ClickableWidget : public W
{
    W_OBJECT(ClickableWidget)
public:
    explicit ClickableWidget(QWidget* parent = nullptr) : W(parent) {}

    bool clickable() const { return m_clickable; }
    void setClickable(bool clickable) { m_clickable = clickable; }

    bool underlineOnHover() const { return m_underlineOnHover; }
    void setUnderlineOnHover(bool underline) { m_underlineOnHover = underline; }

    void makeUnderlined(bool underline)
    {
        QFont newFont(W::font());
        newFont.setUnderline(underline);
        W::setFont(newFont);
    }

    void clicked() W_SIGNAL(clicked)
protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent* event) override
#else
    void enterEvent(QEvent* event) override
#endif
    {
        if (m_clickable)
            W::setCursor(QCursor(Qt::PointingHandCursor));
        if (m_underlineOnHover && !W::font().underline())
            makeUnderlined(true);
        W::enterEvent(event);
    }

    void leaveEvent(QEvent* event) override
    {
        if (m_clickable)
            W::unsetCursor();
        if (m_underlineOnHover && W::font().underline())
            makeUnderlined(false);
        W::leaveEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent* event) override
    {
        if (m_clickable && event->button() == Qt::LeftButton && W::rect().contains(event->pos()))
            emit clicked();
        W::mouseReleaseEvent(event);
    }
private:
    bool m_clickable{};
    bool m_underlineOnHover{};
};

W_OBJECT_IMPL_INLINE(ClickableWidget<W>, template<std::derived_from<QWidget> W>)
