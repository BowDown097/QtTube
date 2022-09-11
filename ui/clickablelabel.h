#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H
#include <QLabel>

class ClickableLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ClickableLabel(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags()) : QLabel(parent, f) {}
    ~ClickableLabel() {}
signals:
    void clicked();
protected:
    void mousePressEvent(QMouseEvent*) override { emit clicked(); }

    void enterEvent(QEnterEvent*) override
    {
        setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        setStyleSheet("QLabel { text-decoration: underline; }");
    }

    void leaveEvent(QEvent*) override
    {
        setCursor(QCursor());
        setStyleSheet("");
    }
};

#endif // CLICKABLELABEL_H
