#pragma once
#include "qttube-plugin/objects/badge.h"
#include "tubelabel.h"
#include <QJsonValue>

class PluginEntry;
class QHBoxLayout;

class ChannelLabel : public QWidget
{
    Q_OBJECT
public:
    TubeLabel* text;

    explicit ChannelLabel(PluginEntry* plugin, QWidget* parent = nullptr);
    void addStretch();
    void setInfo(const QString& uploaderId, const QString& uploaderName, const QList<QtTubePlugin::Badge>& badges);
    void setInfo(const QString& uploaderId, const QString& uploaderName);
private:
    QHBoxLayout* m_badgeLayout;
    QString m_channelId;
    QHBoxLayout* m_layout;

    void reset();
private slots:
    void copyChannelUrl();
    void showContextMenu(const QPoint& pos);
};
