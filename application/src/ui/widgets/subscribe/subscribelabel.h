#pragma once
#include "innertube/objects/channel/subscribebutton.h"
#include "innertube/objects/viewmodels/buttonviewmodel.h"
#include "innertube/objects/viewmodels/subscribebuttonviewmodel.h"
#include "qttube-plugin/objects/channel.h"
#include "ui/widgets/clickablewidget.h"
#include <QLabel>

class SubscribeLabel : public ClickableWidget<QLabel>
{
    Q_OBJECT
public:
    explicit SubscribeLabel(QWidget* parent = nullptr);
    void setData(const QtTube::PluginSubscribeButton& data);
    void setSubscribeButton(const InnertubeObjects::Button& button);
    void setSubscribeButton(const InnertubeObjects::ButtonViewModel& buttonViewModel);
    void setSubscribeButton(const InnertubeObjects::SubscribeButton& subscribeButton);
    void setSubscribeButton(const InnertubeObjects::SubscribeButtonViewModel& subscribeViewModel, bool subscribed);
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
