#include "subscribewidget.h"
#include "notificationbell.h"
#include "subscribelabel.h"
#include "ui/widgets/labels/tubelabel.h"
#include <QBoxLayout>

constexpr QLatin1String SubscribersCountStylesheet(R"(
    border: 1px solid #555;
    font-size: 11px;
    line-height: 24px;
    padding: 0 6px 0 4.5px;
    border-radius: 2px;
    text-align: center;
)");

SubscribeWidget::SubscribeWidget(PluginData* plugin, QWidget* parent)
    : QWidget(parent),
      layout(new QHBoxLayout(this)),
      m_notificationBell(new NotificationBell(plugin, this)),
      m_subscribeLabel(new SubscribeLabel(plugin, this)),
      subscribersCountLabel(new TubeLabel(this))
{
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_subscribeLabel->hide();
    layout->addWidget(m_subscribeLabel);

    m_notificationBell->hide();
    layout->addWidget(m_notificationBell);

    subscribersCountLabel->hide();
    subscribersCountLabel->setFixedHeight(24);
    subscribersCountLabel->setStyleSheet(SubscribersCountStylesheet);
    layout->addWidget(subscribersCountLabel);

    connect(m_subscribeLabel, &SubscribeLabel::subscribeStatusChanged, this, [this](bool subscribed)
    {
        if (qsizetype defaultState = m_notificationBell->defaultEnabledStateIndex(); defaultState >= 0)
            m_notificationBell->setVisualState(defaultState);
        m_notificationBell->setVisible(subscribed);
    });
}

void SubscribeWidget::setData(const QtTubePlugin::SubscribeButton& data)
{
    m_subscribeLabel->setData(data);
    m_subscribeLabel->show();

    if (!data.countText.isEmpty())
    {
        subscribersCountLabel->show();
        subscribersCountLabel->setText(data.countText);
        subscribersCountLabel->adjustSize();
    }
    else
    {
        subscribersCountLabel->hide();
    }

    if (!data.notificationBell.states.isEmpty())
    {
        m_notificationBell->setData(data.notificationBell);
        m_notificationBell->setVisible(data.subscribed);
    }
    else
    {
        m_notificationBell->hide();
    }
}
