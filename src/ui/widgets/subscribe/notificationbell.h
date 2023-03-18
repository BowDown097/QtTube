#ifndef NOTIFICATIONBELL_H
#define NOTIFICATIONBELL_H
#include "ui/widgets/labels/tubelabel.h"

class NotificationBell : public TubeLabel
{
public:
    explicit NotificationBell(QWidget* parent = nullptr);
    void setPreferredPalette(const QPalette& pal);
public slots:
    void displayNotificationMenu();
private:
    QPalette preferredPalette;
};

#endif // NOTIFICATIONBELL_H
