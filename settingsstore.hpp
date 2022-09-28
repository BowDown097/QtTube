#ifndef SETTINGSSTORE_HPP
#define SETTINGSSTORE_HPP
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

class SettingsStore
{
public:
    bool condensedViews;
    bool itcCache;
    bool playbackTracking;
    int preferredVolume;
    bool watchtimeTracking;

    static SettingsStore& instance()
    {
        static SettingsStore it;
        return it;
    }

    void initializeFromSettingsFile()
    {
        QFile settingsFile("settings.json");
        if (!settingsFile.open(QFile::ReadOnly | QFile::Text) || settingsFile.size() == 0)
        {
            condensedViews = false;
            itcCache = true;
            playbackTracking = true;
            preferredVolume = 100;
            watchtimeTracking = true;
            saveToSettingsFile();
            return;
        }

        QTextStream in(&settingsFile);
        QJsonObject settingsObj = QJsonDocument::fromJson(in.readAll().toUtf8()).object();
        condensedViews = settingsObj["condensedViews"].toBool();
        itcCache = settingsObj["itcCache"].toBool(true);
        playbackTracking = settingsObj["playbackTracking"].toBool(true);
        preferredVolume = settingsObj["preferredVolume"].toInt(100);
        watchtimeTracking = settingsObj["watchtimeTracking"].toBool(true);
        settingsFile.close();
    }

    void saveToSettingsFile()
    {
        QFile settingsFile("settings.json");
        if (!settingsFile.open(QFile::WriteOnly | QFile::Text))
            return;

        QJsonObject settingsObj {
            { "condensedViews", condensedViews },
            { "itcCache", itcCache },
            { "playbackTracking", playbackTracking },
            { "preferredVolume", preferredVolume },
            { "watchtimeTracking", watchtimeTracking }
        };

        QTextStream out(&settingsFile);
        out << QJsonDocument(settingsObj).toJson(QJsonDocument::Compact);
        settingsFile.close();
    }
};

#endif // SETTINGSSTORE_HPP