#pragma once
#include "ui/widgets/clickablewidget.h"
#include <QLabel>

namespace InnertubeObjects { struct InnertubeString; }

// have to reinvent the wheel (the wheel being ClickableWidget) quite a bit here to
// support our new boundingRect without having to bloat up ClickableWidget a ton

class TubeLabel : public ClickableWidget<QLabel>
{
    Q_OBJECT
public:
    explicit TubeLabel(QWidget* parent = nullptr);
    explicit TubeLabel(const InnertubeObjects::InnertubeString& text, QWidget* parent = nullptr);
    explicit TubeLabel(const QString& text, QWidget* parent = nullptr);

    void setElideMode(Qt::TextElideMode mode) { m_elideMode = mode; }
    void setText(const QString& text);

    QRect alignedRect(QRect rect) const;
    QRect boundingRect() const;
    QRect boundingRectOfLineAt(const QPoint& point) const;
    Qt::TextElideMode elideMode() const { return m_elideMode; }
protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent* event) override;
#else
    void enterEvent(QEvent* event) override;
#endif
    void leaveEvent(QEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
private:
    Qt::TextElideMode m_elideMode = Qt::ElideNone;
    bool m_hasFixedWidth{};
    QList<QRect> m_lineRects;
    QString m_text;

    void calculateAndSetLineRects();
    int textLineWidth() const;
};
