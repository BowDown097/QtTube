#ifndef CHANNELLABEL_H
#define CHANNELLABEL_H
#include "innertube/objects/channel/metadatabadge.h"
#include "tubelabel.h"
#include <QHBoxLayout>

class ChannelLabel : public QWidget
{
    Q_OBJECT
public:
    TubeLabel* text;
    ChannelLabel(QWidget* parent = nullptr);
    void setInfo(const QString& channelName, const QList<InnertubeObjects::MetadataBadge>& badges);
private:
    QHBoxLayout* layout;
};

#endif // CHANNELLABEL_H
