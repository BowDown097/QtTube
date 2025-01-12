#pragma once
#include "basepostrenderer.h"

namespace InnertubeObjects { struct BackstageImage; }

class QHBoxLayout;
class QLabel;
class QVBoxLayout;

namespace InnertubeObjects { struct Post; }

class PostRenderer : public BasePostRenderer
{
    Q_OBJECT
public:
    explicit PostRenderer(QWidget* parent = nullptr);
    void setData(const InnertubeObjects::Post& post);
private:
    QHBoxLayout* body;
    QHBoxLayout* header;
    QVBoxLayout* layout;
    QHBoxLayout* toolbar;

    void setImage(const InnertubeObjects::BackstageImage& image);
private slots:
    void setImageLabelData(QLabel* imageLabel, const HttpReply& reply);
};
