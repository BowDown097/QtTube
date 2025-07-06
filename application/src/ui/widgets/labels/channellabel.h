#pragma once
#include "qttube-plugin/objects/badge.h"
#include "tubelabel.h"
#include <QJsonValue>

class QHBoxLayout;

class ChannelLabel : public QWidget
{
    Q_OBJECT
public:
    TubeLabel* text;
    explicit ChannelLabel(QWidget* parent = nullptr);
    void addStretch();
    void setInfo(const QString& uploaderId, const QString& uploaderName, const QList<QtTube::PluginBadge>& badges);
    void setInfo(const QString& uploaderId, const QString& uploaderName);
private:
    QHBoxLayout* badgeLayout;
    QString channelId;
    QHBoxLayout* layout;
    void reset();
private slots:
    void copyChannelUrl();
    void navigate();
    void showContextMenu(const QPoint& pos);
};
