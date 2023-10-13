#ifndef PAIDMESSAGE_H
#define PAIDMESSAGE_H
#include "httpreply.h"
#include <QJsonValue>
#include <QLabel>
#include <QVBoxLayout>

class PaidMessage : public QWidget
{
public:
    explicit PaidMessage(const QJsonValue& renderer, QWidget* parent = nullptr);
private:
    QLabel* amountLabel;
    QLabel* authorIcon;
    QLabel* authorLabel;
    QWidget* header;
    QHBoxLayout* headerLayout;
    QVBoxLayout* innerHeaderLayout;
    QVBoxLayout* layout;
    QLabel* messageLabel;
private slots:
    void setAuthorIcon(const HttpReply& reply);
};

#endif // PAIDMESSAGE_H
