#ifndef HOMEVIDEORENDERER_H
#define HOMEVIDEORENDERER_H
#include "clickablelabel.h"
#include <objects/video/video.h>
#include <settingsstore.hpp>
#include <QApplication>
#include <QLabel>
#include <QMessageBox>
#include <QProcess>
#include <QVBoxLayout>

class HomeVideoRenderer : public QWidget
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
    HomeVideoRenderer(QWidget* parent = nullptr) : QWidget(parent)
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
        titleLabel->setFont(QFont(QApplication::font().toString(), QApplication::font().pointSize() + 2));
        connect(channelLabel, &ClickableLabel::clicked, this, &HomeVideoRenderer::navigateChannel);
        connect(titleLabel, &ClickableLabel::clicked, this, &HomeVideoRenderer::navigateVideo);
    }

    void setChannelData(const InnertubeObjects::VideoOwner& owner)
    {
        channelId = owner.id;
        channelLabel->setText(owner.name);
    }

    void setVideoData(bool isLive, const QString& length, const QString& publishedTime, const QString& title, const QString& videoId, const QString& viewCount)
    {
        metadataLabel->setText(isLive ? QStringLiteral("LIVE • %1").arg(viewCount) : QStringLiteral("%1 • %2 • %3").arg(length, publishedTime, viewCount));
        titleLabel->setText(title.length() <= 60 ? title : title.left(60) + "…");
        this->videoId = videoId;
    }
public slots:
    void setThumbnail(const QByteArray& data)
    {
        QPixmap pixmap;
        pixmap.loadFromData(data);
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

#endif // HOMEVIDEORENDERER_H
