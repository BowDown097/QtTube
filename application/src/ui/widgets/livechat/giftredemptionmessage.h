#pragma once
#include "qttube-plugin/objects/livechat/items/giftredemptionmessage.h"
#include <QWidget>

class QHBoxLayout;
class TubeLabel;

class GiftRedemptionMessage : public QWidget
{
public:
    GiftRedemptionMessage(const QtTubePlugin::GiftRedemptionMessage& data, QWidget* parent);
private:
    TubeLabel* m_authorLabel;
    QHBoxLayout* m_layout;
    TubeLabel* m_messageLabel;
};
