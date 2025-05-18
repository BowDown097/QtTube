#include "settingsstore.h"
#include "innertube/objects/ad/adslot.h"
#include "innertube/objects/video/compactvideo.h"
#include "innertube/objects/video/reel.h"
#include "innertube/objects/video/video.h"
#include "innertube/objects/viewmodels/lockupviewmodel.h"
#include "innertube/objects/viewmodels/shortslockupviewmodel.h"
#include <QSettings>

bool SettingsStore::channelIsFiltered(const QString& id) const
{
    return !id.isEmpty() && std::ranges::any_of(filteredChannels, [&id](const QString& c) { return c.startsWith(id); });
}

void SettingsStore::initialize()
{
    QSettings settings(configPath(), QSettings::IniFormat);

    // general
    activePlugin = settings.value("activePlugin").toString();
    appStyle = settings.value("appStyle", "Default").toString();
    autoHideTopBar = settings.value("autoHideTopBar", true).toBool();
    condensedCounts = settings.value("condensedCounts", false).toBool();
    darkTheme = settings.value("darkTheme", false).toBool();
    downloadPath = settings.value("downloadPath").toString();
    fullSubs = settings.value("fullSubs", false).toBool();
    imageCaching = settings.value("imageCaching", true).toBool();
    preferLists = settings.value("preferLists", false).toBool();
    returnDislikes = settings.value("returnDislikes", true).toBool();
    // player
    blockAds = settings.value("player/blockAds", true).toBool();
    disable60Fps = settings.value("player/disable60Fps", false).toBool();
    disablePlayerInfoPanels = settings.value("player/disableInfoPanels", false).toBool();
    externalPlayerPath = settings.value("player/externalPlayerPath").toString();
    h264Only = settings.value("player/h264Only", false).toBool();
    preferredQuality = settings.value("player/preferredQuality", 0).value<PlayerQuality>();
    preferredVolume = settings.value("player/preferredVolume", 100).toInt();
    qualityFromPlayer = settings.value("player/qualityFromPlayer", true).toBool();
    restoreAnnotations = settings.value("player/restoreAnnotations", false).toBool();
    vaapi = settings.value("player/vaapi", false).toBool();
    volumeFromPlayer = settings.value("player/volumeFromPlayer", true).toBool();
    // privacy
    playbackTracking = settings.value("privacy/playbackTracking", true).toBool();
    watchtimeTracking = settings.value("privacy/watchtimeTracking", true).toBool();
    // filtering
    filterLength = settings.value("filtering/filterLength", 0).toInt();
    filterLengthEnabled = settings.value("filtering/filterLengthEnabled", false).toBool();
    hideSearchShelves = settings.value("filtering/hideSearchShelves", true).toBool();
    hideShorts = settings.value("filtering/hideShorts", false).toBool();
    hideStreams = settings.value("filtering/hideStreams", false).toBool();
    readIntoStringList(settings, filteredChannels, "filtering/filteredChannels", "id");
    readIntoStringList(settings, filteredTerms, "filtering/filteredTerms", "term");
    // sponsorblock
    showSBToasts = settings.value("sponsorBlock/toasts", true).toBool();
    readIntoStringList(settings, sponsorBlockCategories, "sponsorBlock/categories", "name");
    // dearrow
    deArrow = settings.value("deArrow/enabled", false).toBool();
    deArrowThumbs = settings.value("deArrow/thumbs", true).toBool();
    deArrowTitles = settings.value("deArrow/titles", true).toBool();
}

void SettingsStore::readIntoStringList(QSettings& settings, QStringList& list, const QString& prefix, const QString& key)
{
    list.clear();

    int sz = settings.beginReadArray(prefix);
    for (int i = 0; i < sz; i++)
    {
        settings.setArrayIndex(i);
        list.append(settings.value(key).toString());
    }
    settings.endArray();
}

void SettingsStore::save()
{
    QSettings settings(configPath(), QSettings::IniFormat);

    // general
    settings.setValue("activePlugin", activePlugin);
    settings.setValue("appStyle", appStyle);
    settings.setValue("autoHideTopBar", autoHideTopBar);
    settings.setValue("condensedCounts", condensedCounts);
    settings.setValue("darkTheme", darkTheme);
    settings.setValue("downloadPath", downloadPath);
    settings.setValue("fullSubs", fullSubs);
    settings.setValue("imageCaching", imageCaching);
    settings.setValue("preferLists", preferLists);
    settings.setValue("returnDislikes", returnDislikes);
    // player
    settings.setValue("player/blockAds", blockAds);
    settings.setValue("player/disable60Fps", disable60Fps);
    settings.setValue("player/disableInfoPanels", disablePlayerInfoPanels);
    settings.setValue("player/externalPlayerPath", externalPlayerPath);
    settings.setValue("player/h264Only", h264Only);
    settings.setValue("player/preferredQuality", static_cast<int>(preferredQuality));
    settings.setValue("player/preferredVolume", preferredVolume);
    settings.setValue("player/qualityFromPlayer", qualityFromPlayer);
    settings.setValue("player/restoreAnnotations", restoreAnnotations);
    settings.setValue("player/vaapi", vaapi);
    settings.setValue("player/volumeFromPlayer", volumeFromPlayer);
    // privacy
    settings.setValue("privacy/playbackTracking", playbackTracking);
    settings.setValue("privacy/watchtimeTracking", watchtimeTracking);
    // filtering
    settings.setValue("filtering/filterLength", filterLength);
    settings.setValue("filtering/filterLengthEnabled", filterLengthEnabled);
    settings.setValue("filtering/hideSearchShelves", hideSearchShelves);
    settings.setValue("filtering/hideShorts", hideShorts);
    settings.setValue("filtering/hideStreams", hideStreams);
    writeStringList(settings, filteredChannels, "filtering/filteredChannels", "id");
    writeStringList(settings, filteredTerms, "filtering/filteredTerms", "term");
    // sponsorblock
    settings.setValue("sponsorBlock/toasts", showSBToasts);
    writeStringList(settings, sponsorBlockCategories, "sponsorBlock/categories", "name");
    // dearrow
    settings.setValue("deArrow/enabled", deArrow);
    settings.setValue("deArrow/thumbs", deArrowThumbs);
    settings.setValue("deArrow/titles", deArrowTitles);
}

bool SettingsStore::strHasFilteredTerm(const QString& str) const
{
    return std::ranges::any_of(filteredTerms, [&str](const QString& t) { return str.contains(t, Qt::CaseInsensitive); });
}

bool SettingsStore::videoIsFiltered(const InnertubeObjects::AdSlot& adSlot) const
{
    if (blockAds)
        return true;

    const auto& renderingContent = adSlot.fulfillmentContent.fulfilledLayout.renderingContent;

    QString title;
    if (const auto* displayAd = std::get_if<InnertubeObjects::DisplayAd>(&renderingContent))
        title = displayAd->titleText.text;
    else if (const auto* video = std::get_if<InnertubeObjects::VideoDisplayButtonGroup>(&renderingContent))
        title = video->title.text;

    return strHasFilteredTerm(title);
}

bool SettingsStore::videoIsFiltered(const InnertubeObjects::CompactVideo& compactVideo) const
{
    return channelIsFiltered(compactVideo.owner().id) || strHasFilteredTerm(compactVideo.title.text) ||
           (filterLengthEnabled && !compactVideo.isLive() && QTime(0, 0).secsTo(compactVideo.length()) <= filterLength) ||
           (hideStreams && compactVideo.isLive());
}

bool SettingsStore::videoIsFiltered(const InnertubeObjects::LockupViewModel& lockup) const
{
    if (std::optional<InnertubeObjects::BasicChannel> owner = lockup.owner(); channelIsFiltered(owner->id))
        return true;

    return strHasFilteredTerm(lockup.metadata.title) ||
           (filterLengthEnabled && !lockup.isLive() && QTime(0, 0).secsTo(lockup.length()) <= filterLength) ||
           (hideStreams && lockup.isLive());
}

bool SettingsStore::videoIsFiltered(const InnertubeObjects::Reel& reel) const
{
    return strHasFilteredTerm(reel.headline);
}

bool SettingsStore::videoIsFiltered(const InnertubeObjects::ShortsLockupViewModel& shortsLockup) const
{
    return strHasFilteredTerm(shortsLockup.primaryText);
}

bool SettingsStore::videoIsFiltered(const InnertubeObjects::Video& video) const
{
    return channelIsFiltered(video.ownerId()) || strHasFilteredTerm(video.title.text) ||
           (filterLengthEnabled && !video.isLive() && QTime(0, 0).secsTo(video.length()) <= filterLength) ||
           (hideShorts && video.isReel()) ||
           (hideStreams && video.isLive());
}

void SettingsStore::writeStringList(QSettings& settings, const QStringList& list, const QString& prefix, const QString& key)
{
    settings.beginWriteArray(prefix);
    for (int i = 0; i < list.size(); i++)
    {
        settings.setArrayIndex(i);
        settings.setValue(key, list.at(i));
    }
    settings.endArray();
}
