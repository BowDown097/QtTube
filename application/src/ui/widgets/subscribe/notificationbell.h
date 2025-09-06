#pragma once
#include "qttube-plugin/objects/channel.h"
#include <QToolButton>

struct PluginData;
class QMenu;

class NotificationBell : public QToolButton
{
    Q_OBJECT
public:
    enum class PreferenceButtonState { None = 0, All = 2, Personalized = 3 };
    enum class PreferenceListState { All, Personalized, None };

    explicit NotificationBell(PluginData* plugin, QWidget* parent = nullptr);
    qsizetype defaultEnabledStateIndex() const { return m_defaultEnabledStateIndex; }
    void setData(const QtTubePlugin::NotificationBell& notificationBell);
    void setVisualState(qsizetype index);
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
    PluginData* m_plugin;
private slots:
    void setState(const QtTubePlugin::NotificationState& state);
};
