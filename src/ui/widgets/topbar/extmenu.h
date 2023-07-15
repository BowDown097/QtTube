#ifndef EXTMENU_H
#define EXTMENU_H
#include <QMenu>

// QMenu that does not trigger actions upon switching to them
class ExtMenu : public QMenu
{
    Q_OBJECT
public:
    explicit ExtMenu(QWidget* parent = nullptr) : QMenu(parent) {}
protected:
    void mouseReleaseEvent(QMouseEvent* event) override;
signals:
    void switchActionRequested(QAction* action);
};

#endif // EXTMENU_H
