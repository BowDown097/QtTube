#pragma once
#include <QJsonValue>
#include <QWidget>

class HttpReply;
class QHBoxLayout;
class QLabel;
class QVBoxLayout;
class TubeLabel;

class PaidMessage : public QWidget
{
public:
    explicit PaidMessage(const QJsonValue& renderer, QWidget* parent = nullptr);
private:
    TubeLabel* amountLabel;
    QLabel* authorIcon;
    TubeLabel* authorLabel;
    QWidget* header;
    QHBoxLayout* headerLayout;
    QVBoxLayout* innerHeaderLayout;
    QVBoxLayout* layout;
    TubeLabel* messageLabel;
private slots:
    void setAuthorIcon(const HttpReply& reply);
};
