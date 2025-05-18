#pragma once
#include "plugins/pluginmanager.h"
#include <QWidget>

class QHBoxLayout;
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
    TubeLabel* imageLabel;
    QHBoxLayout* layout;
    QVBoxLayout* metadataLayout;
    TubeLabel* nameLabel;
};
