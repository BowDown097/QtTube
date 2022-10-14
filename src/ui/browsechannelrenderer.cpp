#include "browsechannelrenderer.h"
#include <QApplication>

BrowseChannelRenderer::BrowseChannelRenderer(QWidget* parent) : QWidget(parent)
{
    hbox = new QHBoxLayout;
    descriptionLabel = new QLabel;
    metadataLabel = new QLabel;
    textVbox = new QVBoxLayout;
    thumbLabel = new ClickableLabel;
    titleLabel = new ClickableLabel;

    textVbox->setSpacing(0);
    textVbox->addWidget(titleLabel);
    textVbox->addWidget(metadataLabel);
    textVbox->addWidget(descriptionLabel);

    hbox->addWidget(thumbLabel);
    hbox->addLayout(textVbox, 1);
    setLayout(hbox);

    descriptionLabel->setWordWrap(true);
    thumbLabel->setMinimumSize(1, 1);
    thumbLabel->setScaledContents(true);
    titleLabel->setFont(QFont(QApplication::font().toString(), QApplication::font().pointSize() + 2));

    connect(thumbLabel, &ClickableLabel::clicked, this, &BrowseChannelRenderer::navigateChannel);
    connect(titleLabel, &ClickableLabel::clicked, this, &BrowseChannelRenderer::navigateChannel);
}

void BrowseChannelRenderer::navigateChannel() { qDebug() << "Navigate" << channelId; }

void BrowseChannelRenderer::setData(const QString& channelId, const QString& descriptionSnippet, const QString& name, bool subbed,
                                    QString subCount, const QString& videoCount)
{
    Q_UNUSED(subbed); // TODO: sub button
    if (!subCount.isEmpty() && !videoCount.isEmpty()) subCount += " • ";

    QString metadata = QStringLiteral("%1%2").arg(subCount, videoCount);
    descriptionLabel->setText(descriptionSnippet);
    metadataLabel->setText(metadata);
    titleLabel->setText(name);
    this->channelId = channelId;
}

void BrowseChannelRenderer::setThumbnail(const HttpReply& reply)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    thumbLabel->setPixmap(pixmap.scaled(200, thumbLabel->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}
