#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H
#include <QLabel>

class ClickableLabel : public QLabel
{
    Q_OBJECT
    bool underline;
public:
    explicit ClickableLabel(bool underline = true, QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags())
        : QLabel(parent, f), underline(underline) {}
    ~ClickableLabel() {}
signals:
    void clicked();
protected:
    void mousePressEvent(QMouseEvent*) override { emit clicked(); }

    void enterEvent(QEnterEvent*) override
    {
        setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        if (underline)
            setStyleSheet("QLabel { text-decoration: underline; }");
    }

    void leaveEvent(QEvent*) override
    {
        setCursor(QCursor());
        setStyleSheet("");
    }
};

#endif // CLICKABLELABEL_H
