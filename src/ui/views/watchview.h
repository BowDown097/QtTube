#ifndef WATCHVIEW_H
#define WATCHVIEW_H
#include <QPointer>
#include <QWidget>

namespace InnertubeEndpoints
{
class Next;
class Player;
class UpdatedMetadataResponse;
}

namespace InnertubeObjects
{
class InnertubeString;
class ToggleButtonViewModel;
}

namespace Ui { class WatchView; }

class HttpReply;
class InnertubeException;
class QTimer;

class WatchView : public QWidget
{
    Q_OBJECT
public:
    explicit WatchView(const QString& videoId, int progress = 0, QWidget* parent = nullptr);
    ~WatchView();
    void hotLoadVideo(const QString& videoId, int progress = 0);
protected:
    void resizeEvent(QResizeEvent* event) override;
private:
    QString channelId;
    QPointer<QTimer> metadataUpdateTimer;
    Ui::WatchView* ui;

    // most logic for these 3 methods courtesy of https://github.com/Rehike/Rehike
    QString generateFormattedDescription(const InnertubeObjects::InnertubeString& description);
    void truncateDescriptionUrl(QString& url, bool prefix = false);
    InnertubeObjects::InnertubeString unattributeDescription(const QJsonValue& attributedDescription);

    void updateMetadata(const InnertubeEndpoints::UpdatedMetadataResponse& resp);
private slots:
    void descriptionLinkActivated(const QString& url);
    void likeOrDislike(bool like, const InnertubeObjects::ToggleButtonViewModel& toggleButton);
    void processNext(const InnertubeEndpoints::Next& endpoint);
    void processPlayer(const InnertubeEndpoints::Player& endpoint);
    void setChannelIcon(const HttpReply& reply);
    void setDislikes(const HttpReply& reply);
    void showContextMenu(const QPoint& pos);
signals:
    void loadFailed(const InnertubeException& ie);
    void navigateChannelRequested(const QString& channelId);
};

#endif // WATCHVIEW_H
