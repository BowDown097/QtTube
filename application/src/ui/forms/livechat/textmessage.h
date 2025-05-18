#pragma once
#include <QJsonValue>
#include <QWidget>

class QHBoxLayout;
class QVBoxLayout;
class TubeLabel;

class TextMessage : public QWidget
{
public:
    TextMessage(const QJsonValue& renderer, QWidget* parent);
private:
    TubeLabel* authorIcon;
    TubeLabel* authorLabel;
    QVBoxLayout* contentLayout;
    QHBoxLayout* headerLayout;
    QHBoxLayout* layout;
    TubeLabel* messageLabel;
    TubeLabel* timestampLabel;
};
