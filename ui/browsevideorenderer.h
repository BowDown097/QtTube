#ifndef BROWSEVIDEORENDERER_H
#define BROWSEVIDEORENDERER_H
#include "clickablelabel.h"
#include "httpreply.h"
#include "innertube.hpp"
#ifdef USEMPV
#include "watchview-mpv.h"
#else
#include "watchview-ytp.h"
#endif
#include <QApplication>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QVBoxLayout>

class BrowseVideoRenderer : public QWidget
{
    Q_OBJECT
    QString channelId;
    ClickableLabel* channelLabel;
    QHBoxLayout* hbox;
    QLabel* metadataLabel;
    QVBoxLayout* textVbox;
    QLabel* thumbLabel;
    ClickableLabel* titleLabel;
    QString videoId;
public:
    BrowseVideoRenderer(QWidget* parent = nullptr) : QWidget(parent)
    {
        channelLabel = new ClickableLabel;
        hbox = new QHBoxLayout;
        metadataLabel = new QLabel;
        textVbox = new QVBoxLayout;
        thumbLabel = new QLabel;
        titleLabel = new ClickableLabel;

        textVbox->addWidget(titleLabel);
        textVbox->addWidget(channelLabel);
        textVbox->addWidget(metadataLabel);

        hbox->addWidget(thumbLabel);
        hbox->addLayout(textVbox, 1);
        setLayout(hbox);

        thumbLabel->setMinimumSize(1, 1);
        thumbLabel->setScaledContents(true);
        titleLabel->setFont(QFont(QApplication::font().toString(), QApplication::font().pointSize() + 2));
        connect(channelLabel, &ClickableLabel::clicked, this, &BrowseVideoRenderer::navigateChannel);
        connect(titleLabel, &ClickableLabel::clicked, this, &BrowseVideoRenderer::navigateVideo);
    }

    void setChannelData(const InnertubeObjects::VideoOwner& owner)
    {
        channelId = owner.id;
        channelLabel->setText(owner.name);
    }

    void setVideoData(QString length, QString publishedTime, int progress, const QString& title, const QString& videoId, const QString& viewCount)
    {
        if (!length.isEmpty()) length += " • ";
        if (!publishedTime.isEmpty()) publishedTime += " • ";

        QString progStr = QStringLiteral(" • %1:%2 watched").arg((progress / 60) % 60).arg(progress % 60, 2, 10, QChar('0'));
        QString metadata = progress != 0
                ? QStringLiteral("%1%2%3%4").arg(length, publishedTime, viewCount, progStr)
                : QStringLiteral("%1%2%3").arg(length, publishedTime, viewCount);
        if (metadata.endsWith("• ")) metadata.chop(2);

        metadataLabel->setText(metadata);
        titleLabel->setText(title.length() <= 60 ? title : title.left(60) + "…");
        this->videoId = videoId;
    }
public slots:
    void setThumbnail(const HttpReply& reply)
    {
        QPixmap pixmap;
        pixmap.loadFromData(reply.body());
        thumbLabel->setPixmap(pixmap.scaled(240, thumbLabel->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
private slots:
    void navigateChannel()
    {
        qDebug() << "Navigate" << channelId;
    }

    void navigateVideo()
    {
        try
        {
            WatchView::instance()->loadVideo(InnerTube::instance().get<InnertubeEndpoints::Player>(videoId));
        }
        catch (const InnertubeException& ie)
        {
            QMessageBox::critical(this, "Failed to load video", ie.message());
        }
    }
};

#endif // BROWSEVIDEORENDERER_H
