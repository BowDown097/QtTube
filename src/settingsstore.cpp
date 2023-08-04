#include "settingsstore.h"

SettingsStore* SettingsStore::instance()
{
    std::call_once(m_onceFlag, [] { m_instance = new SettingsStore; });
    return m_instance;
}

void SettingsStore::initializeFromSettingsFile()
{
    QSettings settings(configPath, QSettings::IniFormat);

    // general
    appStyle = settings.value("appStyle", "Default").toString();
    condensedViews = settings.value("condensedViews", false).toBool();
    darkTheme = settings.value("darkTheme", false).toBool();
    fullSubs = settings.value("fullSubs", false).toBool();
    homeShelves = settings.value("homeShelves", false).toBool();
    returnDislikes = settings.value("returnDislikes", true).toBool();
    // player
    disable60Fps = settings.value("player/disable60Fps", false).toBool();
    disablePlayerInfoPanels = settings.value("player/disableInfoPanels", false).toBool();
    h264Only = settings.value("player/h264Only", false).toBool();
    preferredQuality = settings.value("player/preferredQuality", static_cast<int>(PlayerQuality::Auto)).value<PlayerQuality>();
    preferredVolume = settings.value("player/preferredVolume", 100).toInt();
    restoreAnnotations = settings.value("player/restoreAnnotations", false).toBool();
    // privacy
    playbackTracking = settings.value("privacy/playbackTracking", true).toBool();
    watchtimeTracking = settings.value("privacy/watchtimeTracking", true).toBool();
    // filtering
    hideShorts = settings.value("filtering/hideShorts", false).toBool();
    hideStreams = settings.value("filtering/hideStreams", false).toBool();
    readIntoStringList(settings, filteredChannels, "filtering/filteredChannels", "id");
    readIntoStringList(settings, filteredTerms, "filtering/filteredTerms", "term");
    // sponsorblock
    showSBToasts = settings.value("sponsorBlock/toasts", true).toBool();
    readIntoStringList(settings, sponsorBlockCategories, "sponsorBlock/categories", "name");
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

void SettingsStore::saveToSettingsFile()
{
    QSettings settings(configPath, QSettings::IniFormat);

    // general
    settings.setValue("appStyle", appStyle);
    settings.setValue("condensedViews", condensedViews);
    settings.setValue("darkTheme", darkTheme);
    settings.setValue("fullSubs", fullSubs);
    settings.setValue("homeShelves", homeShelves);
    settings.setValue("returnDislikes", returnDislikes);
    // player
    settings.setValue("player/disable60Fps", disable60Fps);
    settings.setValue("player/disableInfoPanels", disablePlayerInfoPanels);
    settings.setValue("player/h264Only", h264Only);
    settings.setValue("player/preferredQuality", static_cast<int>(preferredQuality));
    settings.setValue("player/preferredVolume", preferredVolume);
    settings.setValue("player/restoreAnnotations", restoreAnnotations);
    // privacy
    settings.setValue("privacy/playbackTracking", playbackTracking);
    settings.setValue("privacy/watchtimeTracking", watchtimeTracking);
    // filtering
    settings.setValue("filtering/hideShorts", hideShorts);
    settings.setValue("filtering/hideStreams", hideStreams);
    writeStringList(settings, filteredChannels, "filtering/filteredChannels", "id");
    writeStringList(settings, filteredTerms, "filtering/filteredTerms", "term");
    // sponsorblock
    settings.setValue("sponsorBlock/toasts", showSBToasts);
    writeStringList(settings, sponsorBlockCategories, "sponsorBlock/categories", "name");
}

bool SettingsStore::stringContainsFilteredTerm(const QString& str)
{
    return std::ranges::any_of(filteredTerms, [&str](const QString& term) { return str.contains(term, Qt::CaseInsensitive); });
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
