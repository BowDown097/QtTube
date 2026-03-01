#pragma once
#include "qttube-plugin/objects/channel.h"
#include "ui/widgets/wrappers/clickablewidget.h"
#include <QLabel>

struct PluginEntry;

class SubscribeLabel : public ClickableWidget<QLabel>
{
    Q_OBJECT
public:
    explicit SubscribeLabel(PluginEntry* plugin, QWidget* parent = nullptr);
    void setData(const QtTubePlugin::SubscribeButton& data);
protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent* event) override;
#else
    void enterEvent(QEvent* event) override;
#endif
    void leaveEvent(QEvent* event) override;
private:
    QtTubePlugin::SubscribeButton m_data;

    void setStyle(bool hovered);
    void toggleSubscriptionStatus();
private slots:
    void trySubscribe(PluginEntry* plugin);
signals:
    void subscribeStatusChanged(bool subscribed);
};
