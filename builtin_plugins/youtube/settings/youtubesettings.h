#pragma once
#include "qttube-plugin/components/settings/settingsstore.h"

namespace InnertubeObjects
{
struct AdSlot;
struct CompactVideo;
struct DisplayAd;
struct LockupViewModel;
struct Reel;
struct ShortsLockupViewModel;
struct Video;
}

class YouTubeSettings : public QObject, public QtTubePlugin::SettingsStore
{
    Q_OBJECT
    Q_PROPERTY(bool disablePlayerInfoPanels MEMBER disablePlayerInfoPanels NOTIFY disablePlayerInfoPanelsChanged)
    Q_PROPERTY(YouTubeSettings::PlayerQuality preferredQuality MEMBER preferredQuality NOTIFY preferredQualityChanged)
    Q_PROPERTY(int preferredVolume MEMBER preferredVolume NOTIFY preferredVolumeChanged)
    Q_PROPERTY(bool qualityFromPlayer MEMBER qualityFromPlayer NOTIFY qualityFromPlayerChanged)
    Q_PROPERTY(bool restoreAnnotations MEMBER restoreAnnotations NOTIFY restoreAnnotationsChanged)
    Q_PROPERTY(QStringList sponsorBlockCategories MEMBER sponsorBlockCategories NOTIFY sponsorBlockCategoriesChanged)
    Q_PROPERTY(bool volumeFromPlayer MEMBER volumeFromPlayer NOTIFY volumeFromPlayerChanged)
public:
    enum class PlayerQuality { Auto, HighRes, HD2160, HD1440, HD1080, HD720, Large, Medium, Small, Tiny };
    Q_ENUM(PlayerQuality)

    bool blockAds{};
    bool condensedCounts{};
    bool disable60Fps{};
    bool disablePlayerInfoPanels{};
    QStringList filteredChannels;
    bool fullSubs{};
    bool h264Only{};
    bool hideSearchShelves{};
    bool hideShorts{};
    bool hideStreams{};
    bool playbackTracking{};
    PlayerQuality preferredQuality{};
    int preferredVolume{};
    bool qualityFromPlayer{};
    bool restoreAnnotations{};
    bool returnDislikes{};
    bool showSBToasts{};
    QStringList sponsorBlockCategories;
    bool volumeFromPlayer{};
    bool watchtimeTracking{};

    void init() override;
    void save() override;
    QtTubePlugin::SettingsWindow* window() override;

    bool channelIsFiltered(const QString& id) const;
    bool videoIsFiltered(const InnertubeObjects::AdSlot& adSlot) const;
    bool videoIsFiltered(const InnertubeObjects::CompactVideo& compactVideo) const;
    bool videoIsFiltered(const InnertubeObjects::DisplayAd&) const { return false; }
    bool videoIsFiltered(const InnertubeObjects::LockupViewModel& lockup) const;
    bool videoIsFiltered(const InnertubeObjects::Reel& reel) const;
    bool videoIsFiltered(const InnertubeObjects::ShortsLockupViewModel& shortsLockup) const;
    bool videoIsFiltered(const InnertubeObjects::Video& video) const;
signals:
    void disablePlayerInfoPanelsChanged(bool);
    void preferredQualityChanged(YouTubeSettings::PlayerQuality);
    void preferredVolumeChanged(int);
    void qualityFromPlayerChanged(bool);
    void restoreAnnotationsChanged(bool);
    void sponsorBlockCategoriesChanged(const QStringList&);
    void volumeFromPlayerChanged(bool);
};
