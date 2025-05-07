#pragma once
#include <QToolButton>

// QToolButton that shows menu on right click
class ExtToolButton : public QToolButton
{
public:
    explicit ExtToolButton(QWidget* parent = nullptr) : QToolButton(parent) {}
protected:
    void mouseReleaseEvent(QMouseEvent* event) override;
};
