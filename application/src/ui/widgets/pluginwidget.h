#pragma once
#include "plugins/pluginmanager.h"
#include <QWidget>

class QHBoxLayout;
class QRadioButton;
class QVBoxLayout;
class TubeLabel;

class PluginWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PluginWidget(PluginData* data, QWidget* parent = nullptr);
    QRadioButton* activeButton() const { return m_activeButton; }
    PluginData* data() const { return m_data; }
private:
    QRadioButton* m_activeButton;
    TubeLabel* m_authorLabel;
    QHBoxLayout* m_buttonsLayout;
    PluginData* m_data;
    TubeLabel* m_descriptionLabel;
    TubeLabel* m_imageLabel;
    QHBoxLayout* m_layout;
    QVBoxLayout* m_metadataLayout;
    TubeLabel* m_nameLabel;
};
