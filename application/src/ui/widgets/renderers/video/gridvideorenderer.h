#pragma once
#include "videorenderer.h"

class QVBoxLayout;

class GridVideoRenderer : public VideoRenderer
{
public:
    explicit GridVideoRenderer(PluginEntry* plugin, QWidget* parent = nullptr);
private:
    QVBoxLayout* m_layout;
};
