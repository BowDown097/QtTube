#include "youtubesettings.h"
#include "youtubesettingsform.h"
#include "innertube/objects/video/compactvideo.h"
#include "innertube/objects/video/video.h"
#include "innertube/objects/viewmodels/lockupviewmodel.h"

void YouTubeSettings::init()
{
    QSettings settings(configPath(), QSettings::IniFormat);

    // general
    condensedCounts = settings.value("condensedCounts", false).toBool();
    fullSubs = settings.value("fullSubs", false).toBool();
    returnDislikes = settings.value("returnDislikes", true).toBool();
    // player
    blockAds = settings.value("player/blockAds", true).toBool();
    disablePlayerInfoPanels = settings.value("player/disableInfoPanels", false).toBool();
    restoreAnnotations = settings.value("player/restoreAnnotations", false).toBool();
    // privacy
    playbackTracking = settings.value("privacy/playbackTracking", true).toBool();
    watchtimeTracking = settings.value("privacy/watchtimeTracking", true).toBool();
    // filtering
    hideSearchShelves = settings.value("filtering/hideSearchShelves", true).toBool();
    hideShorts = settings.value("filtering/hideShorts", false).toBool();
    hideStreams = settings.value("filtering/hideStreams", false).toBool();
    readIntoList(settings, filteredChannels, "filtering/filteredChannels", "id");
    // sponsorblock
    showSBToasts = settings.value("sponsorBlock/toasts", true).toBool();
    readIntoList(settings, sponsorBlockCategories, "sponsorBlock/categories", "name");
}

void YouTubeSettings::save()
{
    QSettings settings(configPath(), QSettings::IniFormat);

    // general
    settings.setValue("condensedCounts", condensedCounts);
    settings.setValue("fullSubs", fullSubs);
    settings.setValue("returnDislikes", returnDislikes);
    // player
    settings.setValue("player/blockAds", blockAds);
    settings.setValue("player/disableInfoPanels", disablePlayerInfoPanels);
    settings.setValue("player/restoreAnnotations", restoreAnnotations);
    // privacy
    settings.setValue("privacy/playbackTracking", playbackTracking);
    settings.setValue("privacy/watchtimeTracking", watchtimeTracking);
    // filtering
    settings.setValue("filtering/hideSearchShelves", hideSearchShelves);
    settings.setValue("filtering/hideShorts", hideShorts);
    settings.setValue("filtering/hideStreams", hideStreams);
    writeList(settings, filteredChannels, "filtering/filteredChannels", "id");
    // sponsorblock
    settings.setValue("sponsorBlock/toasts", showSBToasts);
    writeList(settings, sponsorBlockCategories, "sponsorBlock/categories", "name");
}

QtTubePlugin::SettingsWindow* YouTubeSettings::window()
{
    return new YouTubeSettingsForm;
}

bool YouTubeSettings::channelIsFiltered(const QString& id) const
{
    return !id.isEmpty() && std::ranges::any_of(filteredChannels, [&id](const QString& c) { return c.startsWith(id); });
}

bool YouTubeSettings::videoIsFiltered(const InnertubeObjects::AdSlot& adSlot) const
{
    return blockAds;
}

bool YouTubeSettings::videoIsFiltered(const InnertubeObjects::CompactVideo& compactVideo) const
{
    return channelIsFiltered(compactVideo.owner().id) || (hideStreams && compactVideo.isLive());
}

bool YouTubeSettings::videoIsFiltered(const InnertubeObjects::LockupViewModel& lockup) const
{
    std::optional<InnertubeObjects::BasicChannel> owner = lockup.owner();
    return (owner && channelIsFiltered(owner->id)) || (hideStreams && lockup.isLive());
}

bool YouTubeSettings::videoIsFiltered(const InnertubeObjects::Reel& reel) const
{
    return hideShorts;
}

bool YouTubeSettings::videoIsFiltered(const InnertubeObjects::ShortsLockupViewModel& shortsLockup) const
{
    return hideShorts;
}

bool YouTubeSettings::videoIsFiltered(const InnertubeObjects::Video& video) const
{
    return channelIsFiltered(video.ownerId()) || (hideShorts && video.isReel()) || (hideStreams && video.isLive());
}
