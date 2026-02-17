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
    TubeLabel* m_authorIcon;
    TubeLabel* m_authorLabel;
    QVBoxLayout* m_contentLayout;
    QHBoxLayout* m_headerLayout;
    QHBoxLayout* m_layout;
    TubeLabel* m_messageLabel;
    TubeLabel* m_timestampLabel;
};
