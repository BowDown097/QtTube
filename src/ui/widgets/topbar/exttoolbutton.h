#ifndef EXTTOOLBUTTON_H
#define EXTTOOLBUTTON_H
#include <QToolButton>

// QToolButton that shows menu on right click
class ExtToolButton : public QToolButton
{
public:
    explicit ExtToolButton(QWidget* parent = nullptr) : QToolButton(parent) {}
protected:
    void mousePressEvent(QMouseEvent* event) override;
};

#endif // EXTTOOLBUTTON_H
