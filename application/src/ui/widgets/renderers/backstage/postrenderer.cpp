#include "postrenderer.h"
#include "innertube/objects/backstage/post.h"
#include "qttubeapplication.h"
#include "ui/widgets/labels/channellabel.h"
#include "ui/widgets/labels/iconlabel.h"
#include "ui/widgets/labels/tubelabel.h"
#include "utils/innertubestringformatter.h"
#include "utils/stringutils.h"
#include <QBoxLayout>
#include <QPushButton>

PostRenderer::PostRenderer(QWidget* parent)
    : BasePostRenderer(parent),
      body(new QHBoxLayout),
      bodyContent(new QVBoxLayout),
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
    bodyContent->addWidget(contentText);
    bodyContent->addStretch();
    body->addLayout(bodyContent);

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

    channelLabel->setInfo(channelId, post.authorText.text);
    contentText->setText(InnertubeStringFormatter::formatSimple(post.contentText, false));
    likeLabel->setText(qtTubeApp->settings().condensedCounts
        ? post.voteCount.text : StringUtils::extractDigits(post.actionButtons.likeButton.accessibilityLabel));
    publishedTimeLabel->setText(post.publishedTimeText.text);
    replyLabel->setText(post.actionButtons.replyButton.text.text);

    if (const InnertubeObjects::GenericThumbnail* avatar = post.authorThumbnail.recommendedQuality(channelIconLabel->size()))
        channelIconLabel->setImage("https:" + avatar->url, TubeLabel::Rounded);

    if (auto image = std::get_if<InnertubeObjects::BackstageImage>(&post.backstageAttachment))
        setImage(*image);
    else if (auto poll = std::get_if<InnertubeObjects::Poll>(&post.backstageAttachment))
        setPoll(*poll);
    else if (auto quiz = std::get_if<InnertubeObjects::Quiz>(&post.backstageAttachment))
        setQuiz(*quiz);

    if (const std::optional<InnertubeObjects::Button>& voteButton = post.voteButton; voteButton.has_value())
    {
        QHBoxLayout* buttonLayout = new QHBoxLayout;

        QPushButton* button = new QPushButton(voteButton->text.text, this);
        button->setEnabled(false);
        button->setToolTip("Not implemented yet!");

        buttonLayout->addWidget(button);
        buttonLayout->addStretch();
        bodyContent->addLayout(buttonLayout);
    }
}

void PostRenderer::setImage(const InnertubeObjects::BackstageImage& image)
{
    TubeLabel* imageLabel = new TubeLabel(this);
    imageLabel->setFixedWidth(116);
    imageLabel->setScaledContents(true);
    imageLabel->setMaximumHeight(116);
    body->addWidget(imageLabel);

    if (const InnertubeObjects::GenericThumbnail* bestImage = image.image.bestQuality())
        imageLabel->setImage(bestImage->url);
}

void PostRenderer::setPoll(const InnertubeObjects::Poll& poll)
{
    TubeLabel* votesLabel = new TubeLabel(poll.totalVotes, this);
    votesLabel->setFont(QFont(font().toString(), font().pointSize() - 1));
    bodyContent->addWidget(votesLabel);
}

void PostRenderer::setQuiz(const InnertubeObjects::Quiz& quiz)
{
    TubeLabel* votesLabel = new TubeLabel(quiz.totalVotes.text, this);
    votesLabel->setFont(QFont(font().toString(), font().pointSize() - 1));
    bodyContent->addWidget(votesLabel);
}
