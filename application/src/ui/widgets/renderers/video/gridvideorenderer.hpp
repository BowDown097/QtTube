#pragma once
#include "videorenderer.hpp"

class QVBoxLayout;

class GridVideoRenderer : public VideoRenderer
{
public:
    explicit GridVideoRenderer(PluginEntry* plugin, QWidget* parent = nullptr);
private:
    QVBoxLayout* m_layout;
};
