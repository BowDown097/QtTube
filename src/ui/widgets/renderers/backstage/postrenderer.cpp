#include "postrenderer.h"
#include "http.h"
#include "innertube/objects/backstage/post.h"
#include "qttubeapplication.h"
#include "ui/widgets/labels/channellabel.h"
#include "ui/widgets/labels/iconlabel.h"
#include "ui/widgets/labels/tubelabel.h"
#include "utils/stringutils.h"
#include "utils/uiutils.h"
#include <QBoxLayout>

PostRenderer::PostRenderer(QWidget* parent)
    : BasePostRenderer(parent),
      body(new QHBoxLayout),
      header(new QHBoxLayout),
      layout(new QVBoxLayout(this)),
      toolbar(new QHBoxLayout)
{
    layout->setContentsMargins(0, 0, 0, 0);

    channelIconLabel->setFixedSize(24, 24);
    header->addWidget(channelIconLabel);

    channelLabel->text->setFont(QFont(font().toString(), font().pointSize() - 2));
    header->addWidget(channelLabel);

    publishedTimeLabel->setFont(QFont(font().toString(), font().pointSize() - 2));
    header->addWidget(publishedTimeLabel);
    header->addStretch();

    QWidget* headerWidget = new QWidget;
    headerWidget->setLayout(header);
    layout->addWidget(headerWidget, 0, Qt::AlignTop);

    contentText->setFont(QFont(font().toString(), font().pointSize() - 1));
    body->addWidget(contentText);

    QWidget* bodyWidget = new QWidget;
    bodyWidget->setLayout(body);
    layout->addWidget(bodyWidget, 0, Qt::AlignTop);

    toolbar->addWidget(likeLabel);
    toolbar->addWidget(dislikeLabel);
    toolbar->addStretch();
    toolbar->addWidget(replyLabel);

    QWidget* toolbarWidget = new QWidget;
    toolbarWidget->setLayout(toolbar);
    layout->addWidget(toolbarWidget, 0, Qt::AlignBottom);
}

void PostRenderer::setData(const InnertubeObjects::Post& post)
{
    channelId = post.authorEndpoint["browseEndpoint"]["browseId"].toString();
    postId = post.postId;

    channelLabel->setInfo(channelId, post.authorText.text, {});
    contentText->setText(StringUtils::innertubeStringToRichText(post.contentText, false));
    likeLabel->setText(qtTubeApp->settings().condensedCounts
        ? post.voteCount.text : StringUtils::extractDigits(post.actionButtons.likeButton.accessibilityLabel));
    publishedTimeLabel->setText(post.publishedTimeText.text);
    replyLabel->setText(post.actionButtons.replyButton.text.text);

    if (const InnertubeObjects::GenericThumbnail* avatar = post.authorThumbnail.bestQuality())
    {
        HttpReply* reply = Http::instance().get("https:" + avatar->url);
        connect(reply, &HttpReply::finished, this, &PostRenderer::setChannelIcon);
    }

    if (auto image = std::get_if<InnertubeObjects::BackstageImage>(&post.backstageAttachment))
        setImage(*image);
}

void PostRenderer::setImage(const InnertubeObjects::BackstageImage& image)
{
    QLabel* imageLabel = new QLabel(this);
    imageLabel->setFixedWidth(116);
    imageLabel->setScaledContents(true);
    imageLabel->setMaximumHeight(116);
    body->addWidget(imageLabel);

    if (const InnertubeObjects::GenericThumbnail* bestImage = image.image.bestQuality())
    {
        HttpReply* reply = Http::instance().get(bestImage->url);
        connect(reply, &HttpReply::finished, this, std::bind_front(&PostRenderer::setImageLabelData, this, imageLabel));
    }
}

void PostRenderer::setImageLabelData(QLabel* imageLabel, const HttpReply& reply)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    imageLabel->setPixmap(pixmap);
}
