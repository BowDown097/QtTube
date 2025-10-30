#include "settingsstore.h"
#include "qttube-plugin/plugininterface.h"
#include <QDir>
#include <QSettings>
#include <QStandardPaths>
#include <QTimer>

SettingsStore::SettingsStore(QObject* parent)
    : QObject(parent),
      m_saveDebounceTimer(new QTimer(this))
{
    setConfigPath(resolveConfigPath("", "settings", QtTubePlugin::isPortableBuild()));
    m_saveDebounceTimer->setInterval(500);
    m_saveDebounceTimer->setSingleShot(true);
    connect(m_saveDebounceTimer, &QTimer::timeout, this, &SettingsStore::save);
}

void SettingsStore::init()
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
    playerSettings.h264Only = settings.value("player/h264Only").toBool();
    playerSettings.preferredQuality = static_cast<QtTubePlugin::PlayerSettings::Quality>(
        settings.value("player/preferredQuality").toInt());
    playerSettings.preferredVolume = settings.value("player/preferredVolume", 100).toInt();
    playerSettings.qualityFromPlayer = settings.value("player/qualityFromPlayer").toBool();
    playerSettings.vaapi = settings.value("player/vaapi").toBool();
    playerSettings.volumeFromPlayer = settings.value("player/volumeFromPlayer").toBool();
    // filtering
    filterLength = settings.value("filtering/filterLength", 0).toInt();
    filterLengthEnabled = settings.value("filtering/filterLengthEnabled", false).toBool();
    readIntoList(settings, filteredTerms, "filtering/filteredTerms", "term");

    connect(&playerSettings, &QtTubePlugin::PlayerSettings::preferredQualityChanged, this, [this] {
        m_saveDebounceTimer->start();
    });
    connect(&playerSettings, &QtTubePlugin::PlayerSettings::preferredVolumeChanged, this, [this] {
        m_saveDebounceTimer->start();
    });
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
    settings.setValue("player/h264Only", playerSettings.h264Only);
    settings.setValue("player/preferredQuality", static_cast<int>(playerSettings.preferredQuality));
    settings.setValue("player/preferredVolume", playerSettings.preferredVolume);
    settings.setValue("player/qualityFromPlayer", playerSettings.qualityFromPlayer);
    settings.setValue("player/vaapi", playerSettings.vaapi);
    settings.setValue("player/volumeFromPlayer", playerSettings.volumeFromPlayer);
    // filtering
    settings.setValue("filtering/filterLength", filterLength);
    settings.setValue("filtering/filterLengthEnabled", filterLengthEnabled);
    writeList(settings, filteredTerms, "filtering/filteredTerms", "term");
}

bool SettingsStore::strHasFilteredTerm(const QString& str) const
{
    return std::ranges::any_of(filteredTerms, [&str](const QString& t) { return str.contains(t, Qt::CaseInsensitive); });
}

bool SettingsStore::videoIsFiltered(const QtTubePlugin::Video& video) const
{
    if (strHasFilteredTerm(video.title))
        return true;
    if (filterLengthEnabled)
        if (QTime length = video.length(); length.isValid() && QTime(0, 0).secsTo(length) <= filterLength)
            return true;
    return false;
}
