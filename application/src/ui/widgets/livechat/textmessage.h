#pragma once
#include "qttube-plugin/objects/livechat/items/textmessage.h"
#include <QWidget>

class QHBoxLayout;
class QVBoxLayout;
class TubeLabel;

class TextMessage : public QWidget
{
public:
    TextMessage(const QtTubePlugin::TextMessage& data, QWidget* parent);
private:
    TubeLabel* authorIcon;
    TubeLabel* authorLabel;
    QVBoxLayout* contentLayout;
    QHBoxLayout* headerLayout;
    QHBoxLayout* layout;
    TubeLabel* messageLabel;
    TubeLabel* timestampLabel;
};
