#pragma once
#include "qttube-plugin/objects/channel.h"
#include "ui/widgets/clickablewidget.h"
#include <QLabel>

struct PluginData;

class SubscribeLabel : public ClickableWidget<QLabel>
{
    Q_OBJECT
public:
    explicit SubscribeLabel(PluginData* plugin, QWidget* parent = nullptr);
    void setData(const QtTubePlugin::SubscribeButton& data);
protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent* event) override;
#else
    void enterEvent(QEvent* event) override;
#endif
    void leaveEvent(QEvent* event) override;
private:
    QtTubePlugin::SubscribeButton::ColorPalette colorPalette;
    bool enabled{};
    QtTubePlugin::SubscribeButton::Localization localization;
    std::any subscribeData;
    bool subscribed{};
    std::any unsubscribeData;

    void setStyle(bool hovered);
    void toggleSubscriptionStatus();
private slots:
    void trySubscribe(PluginData* plugin);
signals:
    void subscribeStatusChanged(bool subscribed);
};
