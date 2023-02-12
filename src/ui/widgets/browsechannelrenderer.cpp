#include "browsechannelrenderer.h"
#include "http.h"
#include "innertube/innertubeexception.h"
#include "settingsstore.h"
#include "ui/forms/mainwindow.h"
#include "ui/uiutilities.h"
#include "ui/views/channelview.h"
#include <QApplication>
#include <QMenu>
#include <QMessageBox>

BrowseChannelRenderer::BrowseChannelRenderer(QWidget* parent) : QWidget(parent)
{
    hbox = new QHBoxLayout;
    descriptionLabel = new TubeLabel;
    metadataLabel = new TubeLabel;
    textVbox = new QVBoxLayout;
    thumbLabel = new TubeLabel;
    titleLabel = new TubeLabel;

    textVbox->setSpacing(0);
    textVbox->addWidget(titleLabel);
    textVbox->addWidget(metadataLabel);
    textVbox->addWidget(descriptionLabel);

    hbox->addWidget(thumbLabel);
    hbox->addLayout(textVbox, 1);
    setLayout(hbox);

    descriptionLabel->setWordWrap(true);

    thumbLabel->setClickable(true, false);
    thumbLabel->setMinimumSize(1, 1);
    thumbLabel->setScaledContents(true);

    titleLabel->setClickable(true, true);
    titleLabel->setContextMenuPolicy(Qt::CustomContextMenu);
    titleLabel->setFont(QFont(qApp->font().toString(), qApp->font().pointSize() + 2));

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
    try
    {
        if (MainWindow::centralWidget()->currentIndex() == 2)
            ChannelView::instance()->hotLoadChannel(channelId);
        else
            ChannelView::instance()->loadChannel(channelId);
    }
    catch (const InnertubeException& ie)
    {
        QMessageBox::critical(this, "Failed to load channel", ie.message());
    }
}

void BrowseChannelRenderer::setData(const QString& channelId, const QString& descriptionSnippet, const QString& name, bool subbed,
                                    const QString& subCount, const QString& videoCount)
{
    Q_UNUSED(subbed); // TODO: sub button
    if (descriptionSnippet.isEmpty())
        textVbox->removeWidget(descriptionLabel);
    else
        descriptionLabel->setText(descriptionSnippet);

    UIUtilities::setMaximumLines(descriptionLabel, 2);
    titleLabel->setText(name);
    this->channelId = channelId;

    if (SettingsStore::instance().fullSubs)
    {
        Http http;
        http.setReadTimeout(2000);
        http.setMaxRetries(5);

        HttpReply* reply = http.get(QUrl("https://api.socialcounts.org/youtube-live-subscriber-count/" + channelId));
        connect(reply, &HttpReply::finished, this, [this, videoCount](const HttpReply& reply) {
            int subs = QJsonDocument::fromJson(reply.body())["est_sub"].toInt();
            QString fullSubs = QLocale::system().toString(subs) + " subscribers";
            metadataLabel->setText(QStringLiteral("%1 • %2").arg(fullSubs, videoCount));
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
    thumbLabel->setPixmap(pixmap.scaled(200, thumbLabel->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void BrowseChannelRenderer::showContextMenu(const QPoint& pos)
{
    QMenu* menu = new QMenu(this);

    QAction* copyUrlAction = new QAction("Copy channel page URL", this);
    connect(copyUrlAction, &QAction::triggered, this, &BrowseChannelRenderer::copyChannelUrl);

    menu->addAction(copyUrlAction);
    menu->popup(titleLabel->mapToGlobal(pos));
}
