#include "youtubesettings.h"
#include "youtubesettingsform.h"

void YouTubeSettings::init()
{
    QSettings settings(configPath(), QSettings::IniFormat);

    // general
    condensedCounts = settings.value("condensedCounts", false).toBool();
    fullSubs = settings.value("fullSubs", false).toBool();
    returnDislikes = settings.value("returnDislikes", true).toBool();
    // player
    blockAds = settings.value("player/blockAds", true).toBool();
    disable60Fps = settings.value("player/disable60Fps", false).toBool();
    disablePlayerInfoPanels = settings.value("player/disableInfoPanels", false).toBool();
    h264Only = settings.value("player/h264Only", false).toBool();
    preferredQuality = settings.value("player/preferredQuality", 0).value<PlayerQuality>();
    preferredVolume = settings.value("player/preferredVolume", 100).toInt();
    qualityFromPlayer = settings.value("player/qualityFromPlayer", true).toBool();
    restoreAnnotations = settings.value("player/restoreAnnotations", false).toBool();
    volumeFromPlayer = settings.value("player/volumeFromPlayer", true).toBool();
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
    // dearrow
    deArrow = settings.value("deArrow/enabled", false).toBool();
    deArrowThumbs = settings.value("deArrow/thumbs", true).toBool();
    deArrowTitles = settings.value("deArrow/titles", true).toBool();
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
    settings.setValue("player/disable60Fps", disable60Fps);
    settings.setValue("player/disableInfoPanels", disablePlayerInfoPanels);
    settings.setValue("player/h264Only", h264Only);
    settings.setValue("player/preferredQuality", static_cast<int>(preferredQuality));
    settings.setValue("player/preferredVolume", preferredVolume);
    settings.setValue("player/qualityFromPlayer", qualityFromPlayer);
    settings.setValue("player/restoreAnnotations", restoreAnnotations);
    settings.setValue("player/volumeFromPlayer", volumeFromPlayer);
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
    // dearrow
    settings.setValue("deArrow/enabled", deArrow);
    settings.setValue("deArrow/thumbs", deArrowThumbs);
    settings.setValue("deArrow/titles", deArrowTitles);
}

QtTube::PluginSettingsWindow* YouTubeSettings::window()
{
    return new YouTubeSettingsForm(this);
}
