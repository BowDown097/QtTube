#ifndef GRIDVIDEORENDERER_H
#define GRIDVIDEORENDERER_H
#include "videorenderer.h"
#include <QVBoxLayout>

class GridVideoRenderer : public VideoRenderer
{
public:
    explicit GridVideoRenderer(QWidget* parent = nullptr);
private:
    QVBoxLayout* vbox;
};

#endif // GRIDVIDEORENDERER_H
