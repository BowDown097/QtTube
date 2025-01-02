#pragma once
#include <QToolButton>

class CloseButton : public QToolButton
{
public:
    explicit CloseButton(QWidget* parent = nullptr);
private:
    QAction* action;
};
