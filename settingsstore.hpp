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
    int preferredVolume;

    static SettingsStore& instance()
    {
        static SettingsStore it;
        return it;
    }

    void initializeFromSettingsFile()
    {
        QFile settingsFile("settings.json");
        if (!settingsFile.open(QFile::ReadOnly | QFile::Text) || settingsFile.size() == 0)
            return;

        QTextStream in(&settingsFile);
        QJsonObject settingsObj = QJsonDocument::fromJson(in.readAll().toUtf8()).object();
        condensedViews = settingsObj["condensedViews"].toBool();
        itcCache = settingsObj["itcCache"].toBool(true);
        preferredVolume = settingsObj["preferredVolume"].toInt(100);
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
            { "preferredVolume", preferredVolume }
        };

        QTextStream out(&settingsFile);
        out << QJsonDocument(settingsObj).toJson(QJsonDocument::Compact);
        settingsFile.close();
    }
};

#endif // SETTINGSSTORE_HPP
