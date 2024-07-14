#include "backstagepostrenderer.h"
#include "backstagepollrenderer.h"
#include "http.h"
#include "innertube/objects/backstage/backstagepost.h"
#include "qttubeapplication.h"
#include "ui/views/viewcontroller.h"
#include "ui/widgets/labels/channellabel.h"
#include "ui/widgets/labels/iconlabel.h"
#include "ui/widgets/labels/tubelabel.h"
#include "ui/widgets/renderers/video/browsevideorenderer.h"
#include "utils/stringutils.h"
#include "utils/uiutils.h"
#include <QBoxLayout>
#include <QDesktopServices>
#include <QMenu>
#include <QUrlQuery>

BackstagePostRenderer::BackstagePostRenderer(QWidget* parent)
    : QWidget(parent),
      actionButtons(new QHBoxLayout),
      channelIconLabel(new TubeLabel(this)),
      channelLabel(new ChannelLabel(this)),
      channelTimeLayout(new QHBoxLayout),
      contentText(new TubeLabel(this)),
      dislikeLabel(new IconLabel("dislike")),
      innerLayout(new QVBoxLayout),
      layout(new QHBoxLayout(this)),
      likeLabel(new IconLabel("like")),
      publishedTimeLabel(new TubeLabel(this)),
      readMoreLabel(new TubeLabel(this)),
      replyLabel(new IconLabel("live-chat"))
{
    channelIconLabel->setClickable(true);
    channelIconLabel->setFixedSize(40, 40);
    layout->addWidget(channelIconLabel, 0, Qt::AlignTop);

    channelLabel->text->setFont(QFont(font().toString(), font().pointSize() - 1, QFont::Bold));
    channelTimeLayout->addWidget(channelLabel);

    publishedTimeLabel->setClickable(true);
    publishedTimeLabel->setContextMenuPolicy(Qt::CustomContextMenu);
    publishedTimeLabel->setFont(QFont(font().toString(), font().pointSize() - 2));
    channelTimeLayout->addWidget(publishedTimeLabel);
    channelTimeLayout->addStretch();
    innerLayout->addLayout(channelTimeLayout);

    contentText->setTextFormat(Qt::RichText);
    contentText->setWordWrap(true);
    UIUtils::setMaximumLines(contentText, 3);
    innerLayout->addWidget(contentText);

    readMoreLabel->setClickable(true);
    readMoreLabel->setFont(QFont(font().toString(), -1, QFont::Bold));
    innerLayout->addWidget(readMoreLabel);
    layout->addLayout(innerLayout);

    actionButtons->addWidget(likeLabel);
    actionButtons->addWidget(dislikeLabel);
    actionButtons->addWidget(replyLabel);
    actionButtons->addStretch();

    connect(channelIconLabel, &TubeLabel::clicked, this, &BackstagePostRenderer::navigateChannel);
    connect(channelLabel->text, &TubeLabel::clicked, this, &BackstagePostRenderer::navigateChannel);
    connect(contentText, &TubeLabel::linkActivated, this, &BackstagePostRenderer::linkActivated);
    connect(publishedTimeLabel, &TubeLabel::customContextMenuRequested, this,
            &BackstagePostRenderer::showPublishedTimeContextMenu);
    connect(readMoreLabel, &TubeLabel::clicked, this, &BackstagePostRenderer::toggleReadMore);
}

void BackstagePostRenderer::copyPostUrl()
{
    UIUtils::copyToClipboard("https://www.youtube.com/post/" + postId);
}

void BackstagePostRenderer::linkActivated(const QString& url)
{
    QUrl qUrl(url);
    if (url.startsWith("http"))
    {
        QDesktopServices::openUrl(qUrl);
    }
    else if (url.startsWith("/channel"))
    {
        QString funnyPath = qUrl.path().replace("/channel/", "");
        ViewController::loadChannel(funnyPath.left(funnyPath.indexOf('/')));
    }
    else if (url.startsWith("/watch"))
    {
        QUrlQuery query(qUrl);
        int progress = query.queryItemValue("t").replace("s", "").toInt();
        ViewController::loadVideo(query.queryItemValue("v"), progress);
    }
    else
    {
        qDebug() << "Ran into unsupported description link:" << url;
    }
}

void BackstagePostRenderer::navigateChannel()
{
    ViewController::loadChannel(channelId);
}

void BackstagePostRenderer::setChannelIcon(const HttpReply& reply)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    channelIconLabel->setPixmap(UIUtils::pixmapRounded(pixmap, 20, 20));
}

void BackstagePostRenderer::setData(const InnertubeObjects::BackstagePost& post)
{
    channelId = post.authorEndpoint["browseEndpoint"]["browseId"].toString();
    postId = post.postId;
    readMoreText = post.expandButton.text.text;
    showLessText = post.collapseButton.text.text;
    surface = post.surface;

    channelLabel->setInfo(channelId, post.authorText.text, QList<InnertubeObjects::MetadataBadge>());
    contentText->setText(StringUtils::innertubeStringToRichText(post.contentText, false));
    likeLabel->setText(qtTubeApp->settings().condensedCounts
        ? post.voteCount.text : StringUtils::extractDigits(post.actionButtons.likeButton.accessibilityLabel));
    publishedTimeLabel->setText(post.publishedTimeText.text);
    readMoreLabel->setText(readMoreText);
    readMoreLabel->setVisible(contentText->heightForWidth(width() - channelIconLabel->width()) > contentText->maximumHeight());
    replyLabel->setText(post.actionButtons.replyButton.text.text);

    if (auto recAvatar = post.authorThumbnail.recommendedQuality(channelIconLabel->size()); recAvatar.has_value())
    {
        HttpReply* reply = Http::instance().get("https:" + recAvatar->get().url);
        connect(reply, &HttpReply::finished, this, &BackstagePostRenderer::setChannelIcon);
    }

    if (auto image = std::get_if<InnertubeObjects::BackstageImage>(&post.backstageAttachment))
        setImage(*image);
    else if (auto poll = std::get_if<InnertubeObjects::Poll>(&post.backstageAttachment))
        setPoll(*poll);
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
    if (surface != "BACKSTAGE_SURFACE_TYPE_STREAM")
        imageLabel->setMaximumHeight(420);
    innerLayout->addWidget(imageLabel);

    // workaround for layout fucking up when there's no content
    if (contentText->text().isEmpty())
        contentText->setText(" ");

    if (auto recImage = image.image.recommendedQuality(size()); recImage.has_value())
    {
        HttpReply* reply = Http::instance().get(recImage->get().url);
        connect(reply, &HttpReply::finished, this,
                std::bind(&BackstagePostRenderer::setImageLabelData, this, std::placeholders::_1, imageLabel));
    }
}

void BackstagePostRenderer::setImageLabelData(const HttpReply& reply, QLabel* imageLabel)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    imageLabel->setPixmap(pixmap);
    emit dynamicSizeChange(sizeHint());
}

void BackstagePostRenderer::setPoll(const InnertubeObjects::Poll& poll)
{
    BackstagePollRenderer* pollRenderer = new BackstagePollRenderer(this);
    pollRenderer->setData(poll);
    pollRenderer->setFixedWidth(width());
    innerLayout->addWidget(pollRenderer);
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

void BackstagePostRenderer::showPublishedTimeContextMenu(const QPoint& pos)
{
    QMenu* menu = new QMenu(this);

    QAction* copyUrlAction = new QAction("Copy post URL", this);
    connect(copyUrlAction, &QAction::triggered, this, &BackstagePostRenderer::copyPostUrl);

    menu->addAction(copyUrlAction);
    menu->popup(publishedTimeLabel->mapToGlobal(pos));
}

void BackstagePostRenderer::toggleReadMore()
{
    if (readMoreLabel->text() == readMoreText)
    {
        contentText->setMaximumHeight(QWIDGETSIZE_MAX);
        readMoreLabel->setText(showLessText);
    }
    else
    {
        UIUtils::setMaximumLines(contentText, 3);
        readMoreLabel->setText(readMoreText);
    }

    adjustSize();
    emit dynamicSizeChange(sizeHint());
}
