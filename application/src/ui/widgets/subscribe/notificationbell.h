#pragma once
#include "innertube/objects/channel/notificationpreferencebutton.h"
#include "qttube-plugin/objects/channel.h"
#include <QToolButton>

class QMenu;

class NotificationBell : public QToolButton
{
    Q_OBJECT
public:
    enum class PreferenceButtonState { None = 0, All = 2, Personalized = 3 };
    enum class PreferenceListState { All, Personalized, None };

    explicit NotificationBell(QWidget* parent = nullptr);
    qsizetype defaultEnabledStateIndex() const { return m_defaultEnabledStateIndex; }
    void fromListViewModel(const QJsonValue& listViewModel);
    void fromNotificationPreferenceButton(const InnertubeObjects::NotificationPreferenceButton& npb);
    void setData(const QtTube::PluginNotificationBell& notificationBell);
    void setVisualState(qsizetype index);
    void setVisualState(PreferenceButtonState state);
    void setVisualState(PreferenceListState state);
protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent*) override;
#else
    void enterEvent(QEvent*) override;
#endif
    void leaveEvent(QEvent*) override;
private:
    qsizetype m_defaultEnabledStateIndex = -1;
    QMenu* m_notificationMenu;
    QStringList m_serviceParams;

    void addInnertubeStates();
private slots:
    void setState(PreferenceListState state);
    void setState(const QtTube::PluginNotificationState& state);
};
