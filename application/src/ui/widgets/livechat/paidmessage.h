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
    TubeLabel* m_amountLabel;
    TubeLabel* m_authorIcon;
    TubeLabel* m_authorLabel;
    QWidget* m_header;
    QHBoxLayout* m_headerLayout;
    QVBoxLayout* m_innerHeaderLayout;
    QVBoxLayout* m_layout;
    TubeLabel* m_messageLabel;
};
