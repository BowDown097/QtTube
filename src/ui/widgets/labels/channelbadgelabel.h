#pragma once
#include <QLabel>

namespace InnertubeObjects { class MetadataBadge; }

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
