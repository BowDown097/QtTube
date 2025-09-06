#pragma once
#include "qttube-plugin/objects/badge.h"
#include "tubelabel.h"
#include <QJsonValue>

struct PluginData;
class QHBoxLayout;

class ChannelLabel : public QWidget
{
    Q_OBJECT
public:
    TubeLabel* text;

    explicit ChannelLabel(PluginData* plugin, QWidget* parent = nullptr);
    void addStretch();
    void setInfo(const QString& uploaderId, const QString& uploaderName, const QList<QtTubePlugin::Badge>& badges);
    void setInfo(const QString& uploaderId, const QString& uploaderName);
private:
    QHBoxLayout* badgeLayout;
    QString channelId;
    QHBoxLayout* layout;
    void reset();
private slots:
    void copyChannelUrl();
    void showContextMenu(const QPoint& pos);
};
