#ifndef SETTINGSSTORE_HPP
#define SETTINGSSTORE_HPP
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

class SettingsStore
{
public:
    // QStandardPaths::AppConfigLocation appears to not work in a static context, so we have to make it ourselves :(
    static inline const QDir configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QDir::separator() + "QtTube";
    bool condensedViews;
    bool fullSubs;
    bool homeShelves;
    bool itcCache;
    bool playbackTracking;
    int preferredVolume;
    bool returnDislikes;
    bool showSBToasts;
    QVariantList sponsorBlockCategories;
    bool watchtimeTracking;

    static SettingsStore& instance()
    {
        static SettingsStore it;
        return it;
    }

    void initializeFromSettingsFile()
    {
        QFile settingsFile(configPath.filePath("settings.json"));
        if (!settingsFile.open(QFile::ReadOnly | QFile::Text) || settingsFile.size() == 0)
        {
            condensedViews = false;
            fullSubs = false;
            homeShelves = false;
            itcCache = true;
            playbackTracking = true;
            preferredVolume = 100;
            returnDislikes = true;
            showSBToasts = true;
            watchtimeTracking = true;
            saveToSettingsFile();
            return;
        }

        QTextStream in(&settingsFile);
        QJsonObject settingsObj = QJsonDocument::fromJson(in.readAll().toUtf8()).object();
        condensedViews = settingsObj["condensedViews"].toBool();
        fullSubs = settingsObj["fullSubs"].toBool();
        homeShelves = settingsObj["homeShelves"].toBool();
        itcCache = settingsObj["itcCache"].toBool(true);
        playbackTracking = settingsObj["playbackTracking"].toBool(true);
        preferredVolume = settingsObj["preferredVolume"].toInt(100);
        returnDislikes = settingsObj["returnDislikes"].toBool(true);
        showSBToasts = settingsObj["showSBToasts"].toBool(true);
        sponsorBlockCategories = settingsObj["sponsorBlockCategories"].toArray().toVariantList();
        watchtimeTracking = settingsObj["watchtimeTracking"].toBool(true);
        settingsFile.close();
    }

    void saveToSettingsFile()
    {
        QFile settingsFile(configPath.filePath("settings.json"));
        if (!settingsFile.open(QFile::WriteOnly | QFile::Text))
            return;

        QJsonObject settingsObj {
            { "condensedViews", condensedViews },
            { "fullSubs", fullSubs },
            { "homeShelves", homeShelves },
            { "itcCache", itcCache },
            { "playbackTracking", playbackTracking },
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
};

#endif // SETTINGSSTORE_HPP
