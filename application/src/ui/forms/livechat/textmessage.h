#pragma once
#include <QJsonValue>
#include <QWidget>

class HttpReply;
class QHBoxLayout;
class QLabel;
class QVBoxLayout;
class TubeLabel;

class TextMessage : public QWidget
{
public:
    TextMessage(const QJsonValue& renderer, QWidget* parent);
private:
    QLabel* authorIcon;
    TubeLabel* authorLabel;
    QVBoxLayout* contentLayout;
    QHBoxLayout* headerLayout;
    QHBoxLayout* layout;
    TubeLabel* messageLabel;
    TubeLabel* timestampLabel;
private slots:
    void setAuthorIcon(const HttpReply& reply);
};
