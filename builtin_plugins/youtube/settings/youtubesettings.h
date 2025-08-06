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
    Q_PROPERTY(bool restoreAnnotations MEMBER restoreAnnotations NOTIFY restoreAnnotationsChanged)
    Q_PROPERTY(QStringList sponsorBlockCategories MEMBER sponsorBlockCategories NOTIFY sponsorBlockCategoriesChanged)
public:
    bool blockAds{};
    bool condensedCounts{};
    bool disablePlayerInfoPanels{};
    QStringList filteredChannels;
    bool fullSubs{};
    bool hideSearchShelves{};
    bool hideShorts{};
    bool hideStreams{};
    bool playbackTracking{};
    bool restoreAnnotations{};
    bool returnDislikes{};
    bool showSBToasts{};
    QStringList sponsorBlockCategories;
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
    void restoreAnnotationsChanged(bool);
    void sponsorBlockCategoriesChanged(const QStringList&);
};
