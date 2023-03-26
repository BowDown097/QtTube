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
    void setPreferredPalette(const QPalette& pal);
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
    void updateIcons();

    const QString styles = R"(
    QToolButton {
        border: 1px solid #333;
        border-radius: 2px;
        padding-left: 2px;
        padding-bottom: 1px;
    }

    QToolButton::menu-indicator {
        image: none;
    })";
private slots:
    void updateNotificationState(const QString& iconType);
};

#endif // NOTIFICATIONBELL_H
