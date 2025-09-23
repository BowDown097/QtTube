#pragma once
#include "qttube-plugin/pluginmetadata.h"
#include "ui/widgets/labels/tubelabel.h"

class QHBoxLayout;
class QVBoxLayout;

struct PluginEntryMetadata
{
    QString author;
    QString defaultBranch;
    QString description;
    QString image;
    QString name;
    QString repoFullName;
    QString url;
    QString version;
};

class BasePluginEntry : public QWidget
{
public:
    explicit BasePluginEntry(QWidget* parent = nullptr);
    virtual void setData(const PluginEntryMetadata& metadata);
    void setData(const QtTubePlugin::PluginMetadata& metadata);
protected:
    TubeLabel* m_authorLabel;
    QHBoxLayout* m_buttonsLayout;
    TubeLabel* m_descriptionLabel;
    TubeLabel* m_imageLabel;
    QVBoxLayout* m_layout;
    QVBoxLayout* m_nameAndAuthorLayout;
    TubeLabel* m_nameLabel;
    QHBoxLayout* m_topLayout;
};
