#ifndef BROWSEVIDEORENDERER_H
#define BROWSEVIDEORENDERER_H
#include "videorenderer.h"

class QHBoxLayout;
class QVBoxLayout;

class BrowseVideoRenderer : public VideoRenderer
{
public:
    explicit BrowseVideoRenderer(QWidget* parent = nullptr);
private:
    QHBoxLayout* hbox;
    QVBoxLayout* textVbox;
};

#endif // BROWSEVIDEORENDERER_H
