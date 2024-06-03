#pragma once
#include <wobjectimpl.h>
#include <QMouseEvent>
#include <QWidget>

template<typename T>
concept WidgetType = std::derived_from<T, QWidget>;

template<WidgetType W>
class ClickableWidget : public W
{
    W_OBJECT(ClickableWidget)
public:
    explicit ClickableWidget(QWidget* parent = nullptr) : W(parent) {}
    ClickableWidget(bool clickable, bool underline, QWidget* parent = nullptr)
        : W(parent), m_clickable(clickable), m_underline(underline) {}

    void setClickable(bool clickable, bool underline)
    {
        m_clickable = clickable;
        m_underline = underline;
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
        if (m_underline)
            W::setStyleSheet("QLabel { text-decoration: underline; }");
        W::enterEvent(event);
    }

    void leaveEvent(QEvent* event) override
    {
        if (m_clickable)
            W::setCursor(QCursor());
        if (m_underline)
            W::setStyleSheet(QString());
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
    bool m_underline{};
};

W_OBJECT_IMPL_INLINE(ClickableWidget<W>, template<WidgetType W>)
