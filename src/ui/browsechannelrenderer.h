#ifndef BROWSECHANNELRENDERER_H
#define BROWSECHANNELRENDERER_H
#include "httpreply.h"
#include "tubelabel.h"
#include <QVBoxLayout>

class BrowseChannelRenderer : public QWidget
{
    Q_OBJECT
    QString channelId;
    TubeLabel* descriptionLabel;
    QHBoxLayout* hbox;
    TubeLabel* metadataLabel;
    QVBoxLayout* textVbox;
    TubeLabel* thumbLabel;
    TubeLabel* titleLabel;
public:
    explicit BrowseChannelRenderer(QWidget* parent = nullptr);
    void setData(const QString& channelId, const QString& descriptionSnippet, const QString& name, bool subbed, QString subCount, const QString& videoCount);
public slots:
    void setThumbnail(const HttpReply& reply);
private slots:
    void navigateChannel();
};

#endif // BROWSECHANNELRENDERER_H
