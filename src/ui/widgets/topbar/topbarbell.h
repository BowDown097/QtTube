#pragma once
#include "ui/widgets/clickablewidget.h"
#include <QLabel>

class TopBarBell : public ClickableWidget<QWidget>
{
    Q_OBJECT
public:
    QLabel* bell;
    QLabel* count;

    explicit TopBarBell(QWidget* parent = nullptr);
    void updateCount(int unseenCount);
    void updatePixmap(bool hasNotif, const QPalette& pal);
};
