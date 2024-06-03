#pragma once
#include "tubelabel.h"

namespace InnertubeObjects { struct MetadataBadge; }
class QHBoxLayout;

class ChannelLabel : public QWidget
{
    Q_OBJECT
public:
    TubeLabel* text;
    explicit ChannelLabel(QWidget* parent = nullptr);
    void addStretch();
    void setInfo(const QString& channelId, const QString& channelName, const QList<InnertubeObjects::MetadataBadge>& badges);
private:
    QHBoxLayout* badgeLayout;
    QString channelId;
    QHBoxLayout* layout;
    void reset();
private slots:
    void copyChannelUrl();
    void filterThis();
    void showContextMenu(const QPoint& pos);
};
