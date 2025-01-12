#pragma once
#include "basepostrenderer.h"

namespace InnertubeObjects
{
struct BackstageImage;
struct BackstagePost;
struct Poll;
struct Video;
}

class QHBoxLayout;
class QLabel;
class QVBoxLayout;

class BackstagePostRenderer : public BasePostRenderer
{
    Q_OBJECT
public:
    explicit BackstagePostRenderer(QWidget* parent = nullptr);
    void setData(const InnertubeObjects::BackstagePost& post);
private:
    QHBoxLayout* actionButtons;
    QHBoxLayout* channelTimeLayout;
    QVBoxLayout* innerLayout;
    QHBoxLayout* layout;
    TubeLabel* readMoreLabel;
    QString readMoreText;
    QString showLessText;
    QString surface;

    void setImage(const InnertubeObjects::BackstageImage& image);
    void setPoll(const InnertubeObjects::Poll& poll);
    void setVideo(const InnertubeObjects::Video& video);
private slots:
    void setImageLabelData(QLabel* imageLabel, const HttpReply& reply);
    void toggleReadMore();
signals:
    void dynamicSizeChange(const QSize& newSize);
};
