#pragma once
#include <QPointer>
#include <QWidget>

namespace InnertubeEndpoints
{
class Next;
class Player;
struct UpdatedMetadataResponse;
}

namespace InnertubeObjects
{
struct DynamicText;
struct InnertubeString;
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
    std::unique_ptr<Ui::WatchView> ui;

    void processPreloadData(PreloadData::WatchView* preload);
    InnertubeObjects::InnertubeString unattributeDescription(const InnertubeObjects::DynamicText& attributedDescription);
    void updateMetadata(const QString& videoId);
private slots:
    void descriptionLinkActivated(const QString& url);
    void likeOrDislike(bool like, const InnertubeObjects::ToggleButtonViewModel& toggleButton);
    void processNext(const InnertubeEndpoints::Next& endpoint);
    void processPlayer(const InnertubeEndpoints::Player& endpoint);
    void setChannelIcon(const HttpReply& reply);
    void setDislikes(const HttpReply& reply);
signals:
    void loadFailed(const InnertubeException& ie);
};
