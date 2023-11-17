#ifndef CHANNELBADGELABEL_H
#define CHANNELBADGELABEL_H
#include "innertube/objects/channel/metadatabadge.h"
#include <QLabel>

class ChannelBadgeLabel : public QLabel
{
public:
    explicit ChannelBadgeLabel(QWidget* parent = nullptr);
    void setData(const InnertubeObjects::MetadataBadge& badge);
protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent*) override;
#else
    void enterEvent(QEvent*) override;
#endif
    void leaveEvent(QEvent*) override;
};

#endif // CHANNELBADGELABEL_H
