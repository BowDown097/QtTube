#pragma once
#include "plugins/pluginentry.hpp"
#include <QToolButton>
#include <qttube-plugin/objects/channel.h>

class QMenu;

class NotificationBell : public QToolButton
{
    Q_OBJECT
public:
    enum class PreferenceButtonState { None = 0, All = 2, Personalized = 3 };
    enum class PreferenceListState { All, Personalized, None };

    explicit NotificationBell(PluginEntry* plugin, QWidget* parent = nullptr);
    qsizetype defaultEnabledStateIndex() const { return m_defaultEnabledStateIndex; }
    void setData(const QString& channelId, const QtTubePlugin::NotificationBell& notificationBell);
    void setVisualState(qsizetype index);
protected:
    void changeEvent(QEvent* event) override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent*) override;
#else
    void enterEvent(QEvent*) override;
#endif
    void leaveEvent(QEvent*) override;
private:
    QString m_channelId;
    qsizetype m_defaultEnabledStateIndex = -1;
    QList<QString> m_icons;
    QMenu* m_notificationMenu;
    PluginEntry* m_plugin;
private slots:
    void setState(const QtTubePlugin::NotificationState& state);
};
