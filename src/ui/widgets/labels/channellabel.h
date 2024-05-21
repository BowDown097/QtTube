#pragma once
#include "tubelabel.h"

namespace InnertubeObjects { struct MetadataBadge; }
class QHBoxLayout;

class ChannelLabel : public QWidget
{
public:
    TubeLabel* text;
    explicit ChannelLabel(QWidget* parent = nullptr);
    void setInfo(const QString& channelName, const QList<InnertubeObjects::MetadataBadge>& badges);
private:
    QHBoxLayout* badgeLayout;
    QHBoxLayout* layout;
    void reset();
};
