#include "basepostrenderer.h"
#include "ui/views/viewcontroller.h"
#include "ui/widgets/labels/channellabel.h"
#include "ui/widgets/labels/iconlabel.h"
#include "ui/widgets/labels/tubelabel.h"
#include "utils/uiutils.h"
#include <QDesktopServices>
#include <QMenu>
#include <QUrlQuery>

BasePostRenderer::BasePostRenderer(QWidget* parent)
    : channelIconLabel(new TubeLabel(this)),
      channelLabel(new ChannelLabel(this)),
      contentText(new TubeLabel(this)),
      dislikeLabel(new IconLabel("dislike")),
      likeLabel(new IconLabel("like")),
      publishedTimeLabel(new TubeLabel(this)),
      replyLabel(new IconLabel("live-chat"))
{
    channelIconLabel->setClickable(true);
    channelIconLabel->setScaledContents(true);

    publishedTimeLabel->setClickable(true);
    publishedTimeLabel->setContextMenuPolicy(Qt::CustomContextMenu);

    contentText->setElideMode(Qt::TextElideMode::ElideRight);
    contentText->setTextFormat(Qt::RichText);
    contentText->setWordWrap(true);

    connect(channelIconLabel, &TubeLabel::clicked, this, &BasePostRenderer::navigateChannel);
    connect(contentText, &TubeLabel::linkActivated, this, &BasePostRenderer::linkActivated);
    connect(publishedTimeLabel, &TubeLabel::customContextMenuRequested, this, &BasePostRenderer::showPublishedTimeContextMenu);
}

void BasePostRenderer::copyPostUrl()
{
    UIUtils::copyToClipboard("https://www.youtube.com/post/" + postId);
}

void BasePostRenderer::linkActivated(const QString& url)
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

void BasePostRenderer::navigateChannel()
{
    ViewController::loadChannel(channelId);
}

void BasePostRenderer::showPublishedTimeContextMenu(const QPoint& pos)
{
    QMenu* menu = new QMenu(this);

    QAction* copyUrlAction = new QAction("Copy post URL", this);
    connect(copyUrlAction, &QAction::triggered, this, &BasePostRenderer::copyPostUrl);

    menu->addAction(copyUrlAction);
    menu->popup(publishedTimeLabel->mapToGlobal(pos));
}
