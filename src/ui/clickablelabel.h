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
signals:
    void clicked();
protected:
    void enterEvent(QEnterEvent*) override;
    void leaveEvent(QEvent*) override;
    void mousePressEvent(QMouseEvent*) override { emit clicked(); }
};

#endif // CLICKABLELABEL_H
