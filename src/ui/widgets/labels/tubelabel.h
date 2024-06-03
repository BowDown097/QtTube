#pragma once
#include "ui/widgets/clickablewidget.h"
#include <QLabel>

namespace InnertubeObjects { struct InnertubeString; }

class TubeLabel : public ClickableWidget<QLabel>
{
    Q_OBJECT
public:
    explicit TubeLabel(QWidget* parent = nullptr);
    explicit TubeLabel(const InnertubeObjects::InnertubeString& text, QWidget* parent = nullptr);
    explicit TubeLabel(const QString& text, QWidget* parent = nullptr);
    void setElide(Qt::TextElideMode mode) { m_elideMode = mode; }
    void setText(const QString& text);
private:
    Qt::TextElideMode m_elideMode = Qt::ElideNone;
};
