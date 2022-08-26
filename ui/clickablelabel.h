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
};

#endif // CLICKABLELABEL_H
