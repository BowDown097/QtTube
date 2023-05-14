#include "settingsstore.h"
#include <QSettings>

void SettingsStore::initializeFromSettingsFile()
{
    QSettings settings(configPath.filePath("settings.ini"), QSettings::IniFormat);

    appStyle = settings.value("appStyle", "Default").toString();
    condensedViews = settings.value("condensedViews", false).toBool();
    fullSubs = settings.value("fullSubs", false).toBool();
    homeShelves = settings.value("homeShelves", false).toBool();
    returnDislikes = settings.value("returnDislikes", true).toBool();
    themedChannels = settings.value("themedChannels", false).toBool();

#ifdef Q_OS_WIN
    darkThemeWindows = settings.value("darkThemeWindows", false).toBool();
#endif

    disable60Fps = settings.value("player/disable60Fps", false).toBool();
    h264Only = settings.value("player/h264Only", false).toBool();
    preferredQuality = settings.value("player/preferredQuality", PlayerQuality::Auto).value<PlayerQuality>();
    preferredVolume = settings.value("player/preferredVolume", 100).toInt();
    restoreAnnotations = settings.value("player/restoreAnnotations", false).toBool();

    playbackTracking = settings.value("privacy/playbackTracking", true).toBool();
    watchtimeTracking = settings.value("privacy/watchtimeTracking", true).toBool();

    showSBToasts = settings.value("sponsorBlock/toasts", true).toBool();
    sponsorBlockCategories.clear();

    int size = settings.beginReadArray("sponsorBlock/categories");
    for (int i = 0; i < size; ++i)
    {
        settings.setArrayIndex(i);
        sponsorBlockCategories.append(settings.value("name").toString());
    }
    settings.endArray();
}

void SettingsStore::saveToSettingsFile()
{
    QSettings settings(configPath.filePath("settings.ini"), QSettings::IniFormat);

    settings.setValue("appStyle", appStyle);
    settings.setValue("condensedViews", condensedViews);
    settings.setValue("fullSubs", fullSubs);
    settings.setValue("homeShelves", homeShelves);
    settings.setValue("returnDislikes", returnDislikes);
    settings.setValue("themedChannels", themedChannels);

#ifdef Q_OS_WIN
    settings.setValue("darkThemeWindows", darkThemeWindows);
#endif

    settings.setValue("player/disable60Fps", disable60Fps);
    settings.setValue("player/h264Only", h264Only);
    settings.setValue("player/preferredQuality", preferredQuality);
    settings.setValue("player/preferredVolume", preferredVolume);
    settings.setValue("player/restoreAnnotations", restoreAnnotations);

    settings.setValue("privacy/playbackTracking", playbackTracking);
    settings.setValue("privacy/watchtimeTracking", watchtimeTracking);

    settings.setValue("sponsorBlock/toasts", showSBToasts);

    settings.beginWriteArray("sponsorBlock/categories");
    for (int i = 0; i < sponsorBlockCategories.size(); i++)
    {
        settings.setArrayIndex(i);
        settings.setValue("name", sponsorBlockCategories.at(i));
    }
    settings.endArray();
}
