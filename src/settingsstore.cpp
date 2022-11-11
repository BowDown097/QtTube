#include "settingsstore.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

void SettingsStore::initializeFromSettingsFile()
{
    QFile settingsFile(configPath.filePath("settings.json"));
    if (!settingsFile.open(QFile::ReadOnly | QFile::Text) || settingsFile.size() == 0)
    {
        saveToSettingsFile();
        return;
    }

    QTextStream in(&settingsFile);
    QJsonObject settingsObj = QJsonDocument::fromJson(in.readAll().toUtf8()).object();
    condensedViews = settingsObj["condensedViews"].toBool();
    frontPageTab = static_cast<FrontPageTab>(settingsObj["frontPageTab"].toInt());
    fullSubs = settingsObj["fullSubs"].toBool();
    homeShelves = settingsObj["homeShelves"].toBool();
    itcCache = settingsObj["itcCache"].toBool(true);
    playbackTracking = settingsObj["playbackTracking"].toBool(true);
    preferredQuality = static_cast<PlayerQuality>(settingsObj["preferredQuality"].toInt());
    preferredVolume = settingsObj["preferredVolume"].toInt(100);
    returnDislikes = settingsObj["returnDislikes"].toBool(true);
    showSBToasts = settingsObj["showSBToasts"].toBool(true);
    sponsorBlockCategories = settingsObj["sponsorBlockCategories"].toArray().toVariantList();
    watchtimeTracking = settingsObj["watchtimeTracking"].toBool(true);
    settingsFile.close();
}

void SettingsStore::saveToSettingsFile()
{
    QFile settingsFile(configPath.filePath("settings.json"));
    if (!settingsFile.open(QFile::WriteOnly | QFile::Text))
        return;

    QJsonObject settingsObj {
        { "condensedViews", condensedViews },
        { "frontPageTab", frontPageTab },
        { "fullSubs", fullSubs },
        { "homeShelves", homeShelves },
        { "itcCache", itcCache },
        { "playbackTracking", playbackTracking },
        { "preferredQuality", preferredQuality },
        { "preferredVolume", preferredVolume },
        { "returnDislikes", returnDislikes },
        { "showSBToasts", showSBToasts },
        { "sponsorBlockCategories", QJsonArray::fromVariantList(sponsorBlockCategories) },
        { "watchtimeTracking", watchtimeTracking }
    };

    QTextStream out(&settingsFile);
    out << QJsonDocument(settingsObj).toJson(QJsonDocument::Compact);
    settingsFile.close();
}
