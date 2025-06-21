#pragma once
#include "qttube-plugin/objects/livechat/items/specialmessage.h"
#include <QWidget>

class QVBoxLayout;
class TubeLabel;

class SpecialMessage : public QWidget
{
public:
    explicit SpecialMessage(const QtTube::SpecialMessage& data, QWidget* parent = nullptr);
private:
    TubeLabel* header;
    QVBoxLayout* layout;
    TubeLabel* subtext;
};
