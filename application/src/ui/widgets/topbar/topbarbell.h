#pragma once
#include "ui/widgets/clickablewidget.h"

class QLabel;

class TopBarBell : public ClickableWidget<>
{
    Q_OBJECT
public:
    explicit TopBarBell(QWidget* parent = nullptr);
    QString countText() const;
    bool hasNotifications() const;
    void updateCount(int unseenCount);
    void updatePixmap(bool hasNotif);
private:
    QLabel* m_bell;
    QLabel* m_count;
};
