#ifndef TUBELABEL_H
#define TUBELABEL_H
#include <QLabel>

class TubeLabel : public QLabel
{
    Q_OBJECT
public:
    TubeLabel(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    TubeLabel(const QString& text, QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    void setClickable(bool clickable, bool underline) { this->clickable = clickable; this->underline = underline; }
signals:
    void clicked();
protected:
    void enterEvent(QEnterEvent*) override;
    void leaveEvent(QEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
private:
    bool clickable = false;
    bool underline = false;
};

#endif // TUBELABEL_H
