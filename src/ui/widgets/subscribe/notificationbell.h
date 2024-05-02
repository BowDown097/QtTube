#ifndef NOTIFICATIONBELL_H
#define NOTIFICATIONBELL_H
#include "innertube/objects/channel/notificationpreferencebutton.h"
#include <QToolButton>

class QMenu;

class NotificationBell : public QToolButton
{
    Q_OBJECT
public:
    enum class PreferenceButtonState { None = 0, All = 2, Personalized = 3 };
    enum class PreferenceListState { All, Personalized, None };

    explicit NotificationBell(QWidget* parent = nullptr);
    void fromListViewModel(const QJsonValue& listViewModel);
    void fromNotificationPreferenceButton(const InnertubeObjects::NotificationPreferenceButton& npb);
    void setVisualNotificationState(PreferenceButtonState state);
    void setVisualNotificationState(PreferenceListState state);
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
    QAction* personalizedAction;
    QStringList serviceParams;
private slots:
    void updateNotificationState(PreferenceListState state);
};

#endif // NOTIFICATIONBELL_H
