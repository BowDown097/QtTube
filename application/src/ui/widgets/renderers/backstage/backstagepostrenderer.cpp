#include "backstagepostrenderer.h"
#include "http.h"
#include "innertube/objects/backstage/backstagepost.h"
#include "qttubeapplication.h"
#include "ui/widgets/labels/channellabel.h"
#include "ui/widgets/labels/iconlabel.h"
#include "ui/widgets/labels/tubelabel.h"
#include "ui/widgets/renderers/backstage/backstagepollrenderer.h"
#include "ui/widgets/renderers/backstage/backstagequizrenderer.h"
#include "ui/widgets/renderers/video/browsevideorenderer.h"
#include "utils/innertubestringformatter.h"
#include "utils/stringutils.h"
#include <QBoxLayout>

BackstagePostRenderer::BackstagePostRenderer(QWidget* parent)
    : BasePostRenderer(parent),
      actionButtons(new QHBoxLayout),
      channelTimeLayout(new QHBoxLayout),
      innerLayout(new QVBoxLayout),
      layout(new QHBoxLayout(this)),
      readMoreLabel(new TubeLabel(this))
{
    channelIconLabel->setFixedSize(40, 40);
    layout->addWidget(channelIconLabel, 0, Qt::AlignTop);

    channelLabel->text->setFont(QFont(font().toString(), font().pointSize() - 1, QFont::Bold));
    channelTimeLayout->addWidget(channelLabel);

    publishedTimeLabel->setFont(QFont(font().toString(), font().pointSize() - 2));
    channelTimeLayout->addWidget(publishedTimeLabel);
    channelTimeLayout->addStretch();
    innerLayout->addLayout(channelTimeLayout);

    contentText->setMaximumLines(3);
    innerLayout->addWidget(contentText);

    readMoreLabel->setClickable(true);
    readMoreLabel->setFont(QFont(font().toString(), -1, QFont::Bold));
    innerLayout->addWidget(readMoreLabel);
    layout->addLayout(innerLayout);

    actionButtons->addWidget(likeLabel);
    actionButtons->addWidget(dislikeLabel);
    actionButtons->addWidget(replyLabel);
    actionButtons->addStretch();

    connect(readMoreLabel, &TubeLabel::clicked, this, &BackstagePostRenderer::toggleReadMore);
}

void BackstagePostRenderer::setData(const InnertubeObjects::BackstagePost& post)
{
    channelId = post.authorEndpoint["browseEndpoint"]["browseId"].toString();
    postId = post.postId;
    readMoreText = post.expandButton.text.text;
    showLessText = post.collapseButton.text.text;
    surface = post.surface;

    channelLabel->setInfo(channelId, post.authorText.text);
    contentText->setText(InnertubeStringFormatter::formatSimple(post.contentText, false));
    likeLabel->setText(qtTubeApp->settings().condensedCounts
        ? post.voteCount.text : StringUtils::extractDigits(post.actionButtons.likeButton.accessibilityLabel));
    publishedTimeLabel->setText(post.publishedTimeText.text);
    readMoreLabel->setText(readMoreText);
    readMoreLabel->setVisible(contentText->heightForWidth(width() - channelIconLabel->width()) > contentText->maximumHeight());
    replyLabel->setText(post.actionButtons.replyButton.text.text);

    if (const InnertubeObjects::GenericThumbnail* avatar = post.authorThumbnail.bestQuality())
    {
        HttpReply* reply = Http::instance().get("https:" + avatar->url);
        connect(reply, &HttpReply::finished, this, &BackstagePostRenderer::setChannelIcon);
    }

    if (auto image = std::get_if<InnertubeObjects::BackstageImage>(&post.backstageAttachment))
        setImage(*image);
    else if (auto poll = std::get_if<InnertubeObjects::Poll>(&post.backstageAttachment))
        setPoll(*poll);
    else if (auto quiz = std::get_if<InnertubeObjects::Quiz>(&post.backstageAttachment))
        setQuiz(*quiz);
    else if (auto video = std::get_if<InnertubeObjects::Video>(&post.backstageAttachment))
        setVideo(*video);

    innerLayout->addLayout(actionButtons);
    innerLayout->addStretch();
}

void BackstagePostRenderer::setImage(const InnertubeObjects::BackstageImage& image)
{
    QLabel* imageLabel = new QLabel(this);
    imageLabel->setMaximumWidth(width());
    imageLabel->setScaledContents(true);
    imageLabel->setMaximumHeight(surface == "BACKSTAGE_SURFACE_TYPE_STREAM" ? 638 : 420);
    innerLayout->addWidget(imageLabel);

    // workaround for layout fucking up when there's no content
    if (contentText->text().isEmpty())
        contentText->setText(" ");

    if (const InnertubeObjects::GenericThumbnail* bestImage = image.image.bestQuality())
    {
        HttpReply* reply = Http::instance().get(bestImage->url);
        connect(reply, &HttpReply::finished, this,
                std::bind_front(&BackstagePostRenderer::setImageLabelData, this, imageLabel));
    }
}

void BackstagePostRenderer::setImageLabelData(QLabel* imageLabel, const HttpReply& reply)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    imageLabel->setPixmap(pixmap);
    adjustSize();
}

void BackstagePostRenderer::setPoll(const InnertubeObjects::Poll& poll)
{
    BackstagePollRenderer* pollRenderer = new BackstagePollRenderer(this);
    pollRenderer->setData(poll);
    innerLayout->addWidget(pollRenderer);
}

void BackstagePostRenderer::setQuiz(const InnertubeObjects::Quiz& quiz)
{
    BackstageQuizRenderer* quizRenderer = new BackstageQuizRenderer(this);
    quizRenderer->setData(quiz);
    innerLayout->addWidget(quizRenderer);
    connect(quizRenderer, &BackstageQuizRenderer::explanationUpdated, this, &BackstagePostRenderer::adjustSize);
}

void BackstagePostRenderer::setVideo(const InnertubeObjects::Video& video)
{
    if (qtTubeApp->settings().videoIsFiltered(video))
        return;

    BrowseVideoRenderer* videoRenderer = new BrowseVideoRenderer(this);
    videoRenderer->titleLabel->setMaximumWidth(width() - 100);
    videoRenderer->setData(video);
    innerLayout->addWidget(videoRenderer);
}

void BackstagePostRenderer::toggleReadMore()
{
    if (readMoreLabel->text() == readMoreText)
    {
        contentText->setMaximumHeight(QWIDGETSIZE_MAX);
        contentText->setMaximumLines(-1);
        readMoreLabel->setText(showLessText);
    }
    else
    {
        contentText->setMaximumLines(3);
        readMoreLabel->setText(readMoreText);
    }

    adjustSize();
}
