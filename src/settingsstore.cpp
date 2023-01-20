#include "settingsstore.h"
#include <QSettings>

void SettingsStore::initializeFromSettingsFile()
{
    QSettings settings(configPath.filePath("settings.ini"), QSettings::IniFormat);

    condensedViews = settings.value("condensedViews", false).toBool();
    fullSubs = settings.value("fullSubs", false).toBool();
    homeShelves = settings.value("homeShelves", false).toBool();
    preferredQuality = settings.value("preferredQuality", PlayerQuality::Auto).value<PlayerQuality>();
    preferredVolume = settings.value("preferredVolume", 100).toInt();
    returnDislikes = settings.value("returnDislikes", true).toBool();
    themedChannels = settings.value("themedChannels", false).toBool();

    playbackTracking = settings.value("privacy/playbackTracking", true).toBool();
    watchtimeTracking = settings.value("privacy/watchtimeTracking", true).toBool();

    showSBToasts = settings.value("sponsorBlock/toasts", true).toBool();

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

    settings.setValue("condensedViews", condensedViews);
    settings.setValue("fullSubs", fullSubs);
    settings.setValue("homeShelves", homeShelves);
    settings.setValue("preferredQuality", preferredQuality);
    settings.setValue("preferredVolume", preferredVolume);
    settings.setValue("returnDislikes", returnDislikes);
    settings.setValue("themedChannels", themedChannels);

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
