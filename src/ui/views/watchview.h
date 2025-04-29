#pragma once
#include <QPointer>
#include <QWidget>

namespace InnertubeEndpoints
{
struct Next;
struct Player;
struct UpdatedMetadataResponse;
}

namespace InnertubeObjects
{
struct DynamicText;
struct InnertubeString;
struct LiveChat;
struct ToggleButtonViewModel;
}

namespace PreloadData { struct WatchView; }
namespace Ui { class WatchView; }

class HttpReply;
class InnertubeException;
class QTimer;

class WatchView : public QWidget
{
    Q_OBJECT
public:
    explicit WatchView(const QString& videoId, int progress = 0,
                       PreloadData::WatchView* preload = nullptr, QWidget* parent = nullptr);
    ~WatchView();
    void hotLoadVideo(const QString& videoId, int progress = 0, PreloadData::WatchView* preload = nullptr);
protected:
    void resizeEvent(QResizeEvent* event) override;
private:
    QString channelId;
    QPointer<QTimer> metadataUpdateTimer;
    Ui::WatchView* ui;

    void processPreloadData(PreloadData::WatchView* preload);
    InnertubeObjects::InnertubeString unattributeDescription(const InnertubeObjects::DynamicText& attributedDescription);
    void updateMetadata(const QString& videoId);
private slots:
    void descriptionLinkActivated(const QString& url);
    void likeOrDislike(bool like, const InnertubeObjects::ToggleButtonViewModel& toggleButton);
    void openLiveChat(const InnertubeObjects::LiveChat& conversationBar);
    void processNext(const InnertubeEndpoints::Next& endpoint);
    void processPlayer(const InnertubeEndpoints::Player& endpoint);
    void setChannelIcon(const HttpReply& reply);
    void setDislikes(const HttpReply& reply);
signals:
    void loadFailed(const InnertubeException& ie);
};
