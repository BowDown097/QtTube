#ifndef GIFTREDEMPTIONMESSAGE_H
#define GIFTREDEMPTIONMESSAGE_H
#include <QHBoxLayout>
#include <QJsonValue>
#include <QLabel>

class GiftRedemptionMessage : public QWidget
{
public:
    GiftRedemptionMessage(const QJsonValue& renderer, QWidget* parent);
private:
    QLabel* authorLabel;
    QHBoxLayout* layout;
    QLabel* messageLabel;
};

#endif // GIFTREDEMPTIONMESSAGE_H
