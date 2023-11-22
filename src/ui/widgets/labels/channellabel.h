#ifndef CHANNELLABEL_H
#define CHANNELLABEL_H
#include "tubelabel.h"

namespace InnertubeObjects { class MetadataBadge; }
class QHBoxLayout;

class ChannelLabel : public QWidget
{
public:
    TubeLabel* text;
    explicit ChannelLabel(QWidget* parent = nullptr);
    void setInfo(const QString& channelName, const QList<InnertubeObjects::MetadataBadge>& badges);
private:
    QHBoxLayout* layout;
};

#endif // CHANNELLABEL_H
