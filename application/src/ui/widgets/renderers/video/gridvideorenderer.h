#pragma once
#include "videorenderer.h"

class QVBoxLayout;

class GridVideoRenderer : public VideoRenderer
{
public:
    explicit GridVideoRenderer(QWidget* parent = nullptr);
private:
    QVBoxLayout* vbox;
};
