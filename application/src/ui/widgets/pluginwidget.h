#pragma once
#include "plugins/pluginmanager.h"
#include <QWidget>

class HttpReply;
class QHBoxLayout;
class QLabel;
class QVBoxLayout;
class TubeLabel;

class PluginWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PluginWidget(const PluginData* data, QWidget* parent = nullptr);
private:
    TubeLabel* authorLabel;
    QHBoxLayout* buttonsLayout;
    TubeLabel* descriptionLabel;
    QLabel* imageLabel;
    QHBoxLayout* layout;
    QVBoxLayout* metadataLayout;
    TubeLabel* nameLabel;
private slots:
    void setImage(const HttpReply& reply);
};
