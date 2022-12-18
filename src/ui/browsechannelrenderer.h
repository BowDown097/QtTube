#ifndef BROWSECHANNELRENDERER_H
#define BROWSECHANNELRENDERER_H
#include "clickablelabel.h"
#include "httpreply.h"
#include <QLabel>
#include <QVBoxLayout>

class BrowseChannelRenderer : public QWidget
{
    Q_OBJECT
    QString channelId;
    QLabel* descriptionLabel;
    QHBoxLayout* hbox;
    QLabel* metadataLabel;
    QVBoxLayout* textVbox;
    ClickableLabel* thumbLabel;
    ClickableLabel* titleLabel;
public:
    explicit BrowseChannelRenderer(QWidget* parent = nullptr);
    void setData(const QString& channelId, const QString& descriptionSnippet, const QString& name, bool subbed, QString subCount, const QString& videoCount);
public slots:
    void setThumbnail(const HttpReply& reply);
private slots:
    void navigateChannel();
};

#endif // BROWSECHANNELRENDERER_H
