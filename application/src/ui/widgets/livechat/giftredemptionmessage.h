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
    TubeLabel* authorLabel;
    QHBoxLayout* layout;
    TubeLabel* messageLabel;
};
