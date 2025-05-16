#pragma once
#include "qttube-plugin/objects/video.h"
#include "tubelabel.h"
#include <QJsonValue>

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
    void setInfo(const QString& uploaderId, const QString& uploaderName, const QList<QtTube::PluginVideoBadge>& badges);
    void setInfo(const QJsonValue& endpoint, const QString& name);
    void setInfo(const QString& uploaderId, const QString& uploaderName);
private:
    QHBoxLayout* badgeLayout;
    QJsonValue channelEndpoint;
    QString channelId;
    QHBoxLayout* layout;
    void reset();
private slots:
    void copyChannelUrl();
    void filterThis();
    void navigate();
    void showContextMenu(const QPoint& pos);
};
