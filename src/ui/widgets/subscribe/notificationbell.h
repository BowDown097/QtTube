#ifndef NOTIFICATIONBELL_H
#define NOTIFICATIONBELL_H
#include "innertube/objects/channel/notificationpreferencebutton.h"
#include <QMenu>
#include <QToolButton>

class NotificationBell : public QToolButton
{
    Q_OBJECT
public:
    explicit NotificationBell(QWidget* parent = nullptr);
    void setNotificationPreferenceButton(const InnertubeObjects::NotificationPreferenceButton& npb);
    void setVisualNotificationState(int stateId);
protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent*) override;
#else
    void enterEvent(QEvent*) override;
#endif
    void leaveEvent(QEvent*) override;
private:
    QAction* allAction;
    QAction* noneAction;
    QMenu* notificationMenu;
    InnertubeObjects::NotificationPreferenceButton notificationPreferenceButton;
    QAction* personalizedAction;
private slots:
    void updateNotificationState(const QString& iconType);
};

#endif // NOTIFICATIONBELL_H
