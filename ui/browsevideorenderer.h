#ifndef BROWSEVIDEORENDERER_H
#define BROWSEVIDEORENDERER_H
#include "clickablelabel.h"
#include "httpreply.h"
#include "objects/video/videoowner.h"
#include "settingsstore.hpp"
#include <QApplication>
#include <QLabel>
#include <QMessageBox>
#include <QProcess>
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

    void setVideoData(QString length, QString publishedTime, const QString& title, const QString& videoId, const QString& viewCount)
    {
        if (!length.isEmpty()) length += " • ";
        if (!publishedTime.isEmpty()) publishedTime += " • ";

        QString metadata = QStringLiteral("%1%2%3").arg(length, publishedTime, viewCount);
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
        if (SettingsStore::instance().playerPath.isEmpty())
        {
            QMessageBox::warning(nullptr, "No player set!", "You have not set a video player to use. Please set it in the Settings.");
            return;
        }

        QProcess* proc = new QProcess;
        proc->start(SettingsStore::instance().playerPath, QStringList() << SettingsStore::instance().playerArgs << QStringLiteral("https://youtube.com/watch?v=%1").arg(videoId));
    }
};

#endif // BROWSEVIDEORENDERER_H
