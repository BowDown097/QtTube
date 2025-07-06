#pragma once
#include "qttube-plugin/objects/channel.h"
#include "ui/widgets/clickablewidget.h"
#include <QLabel>

class SubscribeLabel : public ClickableWidget<QLabel>
{
    Q_OBJECT
public:
    explicit SubscribeLabel(QWidget* parent = nullptr);
    void setData(const QtTube::PluginSubscribeButton& data);
protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent* event) override;
#else
    void enterEvent(QEvent* event) override;
#endif
    void leaveEvent(QEvent* event) override;
private:
    QtTube::PluginSubscribeButton::ColorPalette colorPalette;
    QtTube::PluginSubscribeButton::Localization localization;
    std::any subscribeData;
    bool subscribed{};
    std::any unsubscribeData;

    void setStyle(bool subscribed, bool hovered);
    void toggleSubscriptionStatus();
private slots:
    void trySubscribe();
signals:
    void subscribeStatusChanged(bool subscribed);
};
