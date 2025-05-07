#pragma once
#include <QJsonValue>
#include <QWidget>

class QHBoxLayout;
class TubeLabel;

class GiftRedemptionMessage : public QWidget
{
public:
    GiftRedemptionMessage(const QJsonValue& renderer, QWidget* parent);
private:
    TubeLabel* authorLabel;
    QHBoxLayout* layout;
    TubeLabel* messageLabel;
};
