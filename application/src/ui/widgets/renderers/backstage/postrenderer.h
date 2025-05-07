#pragma once
#include "basepostrenderer.h"

namespace InnertubeObjects { struct BackstageImage; struct Poll; struct Quiz; }

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
    QVBoxLayout* bodyContent;
    QHBoxLayout* header;
    QVBoxLayout* layout;
    QHBoxLayout* toolbar;

    void setImage(const InnertubeObjects::BackstageImage& image);
    void setPoll(const InnertubeObjects::Poll& poll);
    void setQuiz(const InnertubeObjects::Quiz& quiz);
private slots:
    void setImageLabelData(QLabel* imageLabel, const HttpReply& reply);
};
