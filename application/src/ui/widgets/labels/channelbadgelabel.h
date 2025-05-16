#pragma once
#include "qttube-plugin/objects/video.h"
#include <QLabel>

namespace InnertubeObjects { struct MetadataBadge; }

class ChannelBadgeLabel : public QLabel
{
public:
    explicit ChannelBadgeLabel(QWidget* parent = nullptr);
    void setData(const InnertubeObjects::MetadataBadge& badge);
    void setData(const QtTube::PluginVideoBadge& badge);
protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent*) override;
#else
    void enterEvent(QEvent*) override;
#endif
    void leaveEvent(QEvent*) override;
};
