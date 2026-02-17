#pragma once
#include "videorenderer.h"

class QHBoxLayout;
class QVBoxLayout;

class BrowseVideoRenderer : public VideoRenderer
{
public:
    explicit BrowseVideoRenderer(PluginData* plugin, QWidget* parent = nullptr);
private:
    QHBoxLayout* m_layout;
    QVBoxLayout* m_textLayout;
};
