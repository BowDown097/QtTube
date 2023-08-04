#include "browsechannelrenderer.h"
#include "stores/settingsstore.h"
#include "utils/uiutils.h"
#include "ui/views/viewcontroller.h"
#include <QApplication>
#include <QMenu>
#include <QMessageBox>
#include <QtNetwork/QtNetwork>

BrowseChannelRenderer::BrowseChannelRenderer(QWidget* parent)
    : QWidget(parent),
      descriptionLabel(new TubeLabel(this)),
      hbox(new QHBoxLayout(this)),
      metadataLabel(new TubeLabel(this)),
      subscribeWidget(new SubscribeWidget(this)),
      textVbox(new QVBoxLayout),
      thumbLabel(new TubeLabel(this)),
      titleLabel(new TubeLabel(this))
{
    titleLabel->setClickable(true, true);
    titleLabel->setContextMenuPolicy(Qt::CustomContextMenu);
    titleLabel->setFont(QFont(qApp->font().toString(), qApp->font().pointSize() + 2, QFont::Bold));
    textVbox->addWidget(titleLabel);

    textVbox->addWidget(metadataLabel);

    descriptionLabel->setWordWrap(true);
    UIUtils::setMaximumLines(descriptionLabel, 2);
    textVbox->addWidget(descriptionLabel);

    thumbLabel->setClickable(true, false);
    thumbLabel->setFixedSize(80, 80);
    hbox->addWidget(thumbLabel);

    hbox->addLayout(textVbox, 1);
    hbox->addWidget(subscribeWidget);

    connect(thumbLabel, &TubeLabel::clicked, this, &BrowseChannelRenderer::navigateChannel);
    connect(titleLabel, &TubeLabel::clicked, this, &BrowseChannelRenderer::navigateChannel);
    connect(titleLabel, &TubeLabel::customContextMenuRequested, this, &BrowseChannelRenderer::showContextMenu);
}

void BrowseChannelRenderer::copyChannelUrl()
{
    UIUtils::copyToClipboard("https://www.youtube.com/channel/" + channelId);
}

void BrowseChannelRenderer::navigateChannel()
{
    ViewController::loadChannel(channelId);
}

void BrowseChannelRenderer::setData(const QString& channelId, const QString& descriptionSnippet, const QString& name,
                                    const InnertubeObjects::SubscribeButton& subButton, const QString& subCount, const QString& videoCount)
{
    this->channelId = channelId;

    titleLabel->setText(name);

    if (descriptionSnippet.isEmpty())
    {
        textVbox->removeWidget(descriptionLabel);
        descriptionLabel->deleteLater();
    }
    else
    {
        descriptionLabel->setText(descriptionSnippet);
    }

    subscribeWidget->setSubscribeButton(subButton);
    subscribeWidget->setSubscriberCount(subCount.contains("subscribers") ? subCount : videoCount, channelId);

    if (SettingsStore::instance()->fullSubs)
    {
        // QNetworkAccessManager needs to be used here due to a bug with the http library
        QNetworkAccessManager* manager = new QNetworkAccessManager(this);
        manager->setTransferTimeout(2000);

        QNetworkReply* reply = manager->get(QNetworkRequest(QUrl("https://api.socialcounts.org/youtube-live-subscriber-count/" + channelId)));
        connect(reply, &QNetworkReply::finished, this, [this, reply, subCount, videoCount] {
            reply->deleteLater();
            reply->manager()->deleteLater();

            if (reply->error() != QNetworkReply::NoError)
                return;

            int subs = QJsonDocument::fromJson(reply->readAll())["est_sub"].toInt();
            QString fullSubs = QLocale::system().toString(subs) + " subscribers";

            // in some cases, subCount is the channel handle, and other times it's actually the sub count.
            // thanks innertube! so ya, we have to check for that, otherwise subs show up twice.
            metadataLabel->setText(subCount.contains(" subscribers")
                                   ? QStringLiteral("%1 • %2").arg(fullSubs, videoCount)
                                   : QStringLiteral("%1 • %2").arg(subCount, fullSubs));
        });
    }
    else
    {
        metadataLabel->setText(!videoCount.isEmpty() ? QStringLiteral("%1 • %2").arg(subCount, videoCount) : subCount);
    }
}

void BrowseChannelRenderer::setThumbnail(const HttpReply& reply)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    thumbLabel->setPixmap(pixmap.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void BrowseChannelRenderer::showContextMenu(const QPoint& pos)
{
    QMenu* menu = new QMenu(this);

    QAction* copyUrlAction = new QAction("Copy channel page URL", this);
    connect(copyUrlAction, &QAction::triggered, this, &BrowseChannelRenderer::copyChannelUrl);

    menu->addAction(copyUrlAction);
    menu->popup(titleLabel->mapToGlobal(pos));
}
