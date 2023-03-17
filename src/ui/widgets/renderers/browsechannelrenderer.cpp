#include "browsechannelrenderer.h"
#include "http.h"
#include "settingsstore.h"
#include "ui/uiutilities.h"
#include "ui/views/viewcontroller.h"
#include <QApplication>
#include <QMenu>
#include <QMessageBox>

BrowseChannelRenderer::BrowseChannelRenderer(QWidget* parent) : QWidget(parent)
{
    hbox = new QHBoxLayout(this);
    setLayout(hbox);

    textVbox = new QVBoxLayout(this);

    titleLabel = new TubeLabel(this);
    titleLabel->setClickable(true, true);
    titleLabel->setContextMenuPolicy(Qt::CustomContextMenu);
    titleLabel->setFont(QFont(qApp->font().toString(), qApp->font().pointSize() + 2));
    textVbox->addWidget(titleLabel);

    metadataLabel = new TubeLabel(this);
    textVbox->addWidget(metadataLabel);

    descriptionLabel = new TubeLabel(this);
    descriptionLabel->setWordWrap(true);
    UIUtilities::setMaximumLines(descriptionLabel, 2);
    textVbox->addWidget(descriptionLabel);

    thumbLabel = new TubeLabel(this);
    thumbLabel->setClickable(true, false);
    thumbLabel->setFixedSize(80, 80);

    hbox->addWidget(thumbLabel);
    hbox->addLayout(textVbox, 1);

    connect(thumbLabel, &TubeLabel::clicked, this, &BrowseChannelRenderer::navigateChannel);
    connect(titleLabel, &TubeLabel::clicked, this, &BrowseChannelRenderer::navigateChannel);
    connect(titleLabel, &TubeLabel::customContextMenuRequested, this, &BrowseChannelRenderer::showContextMenu);
}

void BrowseChannelRenderer::copyChannelUrl()
{
    UIUtilities::copyToClipboard("https://www.youtube.com/channel/" + channelId);
}

void BrowseChannelRenderer::navigateChannel()
{
    ViewController::loadChannel(channelId);
}

void BrowseChannelRenderer::setData(const QString& channelId, const QString& descriptionSnippet, const QString& name, bool subbed,
                                    const QString& subCount, const QString& videoCount)
{
    Q_UNUSED(subbed); // TODO: sub button
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

    if (SettingsStore::instance().fullSubs)
    {
        Http http;
        http.setReadTimeout(2000);
        http.setMaxRetries(5);

        HttpReply* reply = http.get(QUrl("https://api.socialcounts.org/youtube-live-subscriber-count/" + channelId));
        connect(reply, &HttpReply::finished, this, [this, subCount, videoCount](const HttpReply& reply) {
            int subs = QJsonDocument::fromJson(reply.body())["est_sub"].toInt();
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
