#ifndef BROWSECHANNELRENDERER_H
#define BROWSECHANNELRENDERER_H
#include "clickablelabel.h"
#include "httpreply.h"
#include <QApplication>
#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>

class BrowseChannelRenderer : public QWidget
{
    Q_OBJECT
    QString channelId;
    QLabel* descriptionLabel;
    QHBoxLayout* hbox;
    QLabel* metadataLabel;
    QVBoxLayout* textVbox;
    QLabel* thumbLabel;
    ClickableLabel* titleLabel;
public:
    explicit BrowseChannelRenderer(QWidget* parent = nullptr) : QWidget(parent)
    {
        hbox = new QHBoxLayout;
        descriptionLabel = new QLabel;
        metadataLabel = new QLabel;
        textVbox = new QVBoxLayout;
        thumbLabel = new QLabel;
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
        connect(titleLabel, &ClickableLabel::clicked, this, &BrowseChannelRenderer::navigateChannel);
    }

    void setData(const QString& channelId, const QString& descriptionSnippet, const QString& name, bool subbed, QString subCount, const QString& videoCount)
    {
        Q_UNUSED(subbed); // TODO: sub button
        if (!subCount.isEmpty() && !videoCount.isEmpty()) subCount += " • ";

        QString metadata = QStringLiteral("%1%2").arg(subCount, videoCount);
        descriptionLabel->setText(descriptionSnippet);
        metadataLabel->setText(metadata);
        titleLabel->setText(name);
        this->channelId = channelId;
    }
public slots:
    void setThumbnail(const HttpReply& reply)
    {
        QPixmap pixmap;
        pixmap.loadFromData(reply.body());
        thumbLabel->setPixmap(pixmap.scaled(200, thumbLabel->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
private slots:
    void navigateChannel()
    {
        qDebug() << "Navigate" << channelId;
    }
};

#endif // BROWSECHANNELRENDERER_H
