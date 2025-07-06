#include "settingsstore.h"
#include <QSettings>

void SettingsStore::initialize()
{
    QSettings settings(configPath(), QSettings::IniFormat);

    // general
    activePlugin = settings.value("activePlugin").toString();
    appStyle = settings.value("appStyle", "Default").toString();
    autoHideTopBar = settings.value("autoHideTopBar", true).toBool();
    darkTheme = settings.value("darkTheme", false).toBool();
    imageCaching = settings.value("imageCaching", true).toBool();
    preferLists = settings.value("preferLists", false).toBool();
    // player
    externalPlayerPath = settings.value("player/externalPlayerPath").toString();
    vaapi = settings.value("player/vaapi", false).toBool();
    // filtering
    filterLength = settings.value("filtering/filterLength", 0).toInt();
    filterLengthEnabled = settings.value("filtering/filterLengthEnabled", false).toBool();
    readIntoStringList(settings, filteredTerms, "filtering/filteredTerms", "term");
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
    settings.setValue("darkTheme", darkTheme);
    settings.setValue("imageCaching", imageCaching);
    settings.setValue("preferLists", preferLists);
    // player
    settings.setValue("player/externalPlayerPath", externalPlayerPath);
    settings.setValue("player/vaapi", vaapi);
    // filtering
    settings.setValue("filtering/filterLength", filterLength);
    settings.setValue("filtering/filterLengthEnabled", filterLengthEnabled);
    writeStringList(settings, filteredTerms, "filtering/filteredTerms", "term");
    // dearrow
    settings.setValue("deArrow/enabled", deArrow);
    settings.setValue("deArrow/thumbs", deArrowThumbs);
    settings.setValue("deArrow/titles", deArrowTitles);
}

bool SettingsStore::strHasFilteredTerm(const QString& str) const
{
    return std::ranges::any_of(filteredTerms, [&str](const QString& t) { return str.contains(t, Qt::CaseInsensitive); });
}

bool SettingsStore::videoIsFiltered(const QtTube::PluginVideo& video) const
{
    if (strHasFilteredTerm(video.title))
        return true;
    if (filterLengthEnabled)
        if (QTime length = video.length(); length.isValid() && QTime(0, 0).secsTo(length) <= filterLength)
            return true;
    return false;
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
