#include "settingsstore.h"
#include "qttube-plugin/plugininterface.h"
#include <QDir>
#include <QSettings>
#include <QStandardPaths>
#include <QTimer>

SettingsStore::SettingsStore(QObject* parent)
    : QObject(parent),
      m_configPath(QtTubePlugin::isPortableBuild()
          ? FS::joinPaths(QCoreApplication::applicationDirPath(), "config", "settings.ini")
          : FS::joinPaths(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation), "settings.ini")),
      m_saveDebounceTimer(new QTimer(this))
{
    m_saveDebounceTimer->setInterval(500);
    m_saveDebounceTimer->setSingleShot(true);
    connect(m_saveDebounceTimer, &QTimer::timeout, this, &SettingsStore::save);
}

void SettingsStore::initialize()
{
    QSettings settings(m_configPath, QSettings::IniFormat);

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
    playerSettings.preferredVolume = settings.value("player/preferredVolume").toInt();
    playerSettings.qualityFromPlayer = settings.value("player/qualityFromPlayer").toBool();
    playerSettings.vaapi = settings.value("player/vaapi").toBool();
    playerSettings.volumeFromPlayer = settings.value("player/volumeFromPlayer").toBool();
    // filtering
    filterLength = settings.value("filtering/filterLength", 0).toInt();
    filterLengthEnabled = settings.value("filtering/filterLengthEnabled", false).toBool();
    readIntoStringList(settings, filteredTerms, "filtering/filteredTerms", "term");

    connect(&playerSettings, &QtTubePlugin::PlayerSettings::preferredQualityChanged, this, [this] {
        m_saveDebounceTimer->start();
    });
    connect(&playerSettings, &QtTubePlugin::PlayerSettings::preferredVolumeChanged, this, [this] {
        m_saveDebounceTimer->start();
    });
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
    QSettings settings(m_configPath, QSettings::IniFormat);

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
    writeStringList(settings, filteredTerms, "filtering/filteredTerms", "term");
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
