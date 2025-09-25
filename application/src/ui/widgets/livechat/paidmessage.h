#pragma once
#include "qttube-plugin/objects/livechat/items/paidmessage.h"
#include <QWidget>

class QHBoxLayout;
class QVBoxLayout;
class TubeLabel;

class PaidMessage : public QWidget
{
public:
    explicit PaidMessage(const QtTubePlugin::PaidMessage& data, QWidget* parent = nullptr);
private:
    TubeLabel* amountLabel;
    TubeLabel* authorIcon;
    TubeLabel* authorLabel;
    QWidget* header;
    QHBoxLayout* headerLayout;
    QVBoxLayout* innerHeaderLayout;
    QVBoxLayout* layout;
    TubeLabel* messageLabel;
};
