#ifndef TEXTMESSAGE_H
#define TEXTMESSAGE_H
#include "httpreply.h"
#include <QHBoxLayout>
#include <QJsonValue>
#include <QLabel>

class TextMessage : public QWidget
{
public:
    TextMessage(const QJsonValue& renderer, QWidget* parent);
private:
    QLabel* authorIcon;
    QLabel* authorLabel;
    QVBoxLayout* contentLayout;
    QHBoxLayout* layout;
    QLabel* messageLabel;
private slots:
    void insertEmojiIntoMessage(const HttpReply& reply, const QString& placeholder);
    void setAuthorIcon(const HttpReply& reply);
};

#endif // TEXTMESSAGE_H
