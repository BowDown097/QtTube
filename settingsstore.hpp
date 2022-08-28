#ifndef SETTINGSSTORE_HPP
#define SETTINGSSTORE_HPP
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

class SettingsStore
{
public:
    bool itcCache = true;
    QString playerArgs;
    QString playerPath;

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
        itcCache = settingsObj["itcCache"].toBool();
        playerArgs = settingsObj["playerArgs"].toString();
        playerPath = settingsObj["playerPath"].toString();
        settingsFile.close();
    }

    void saveToSettingsFile()
    {
        QFile settingsFile("settings.json");
        if (!settingsFile.open(QFile::WriteOnly | QFile::Text))
            return;

        QJsonObject settingsObj {
            { "itcCache", itcCache },
            { "playerArgs", playerArgs },
            { "playerPath", playerPath }
        };

        QTextStream out(&settingsFile);
        out << QJsonDocument(settingsObj).toJson(QJsonDocument::Compact);
        settingsFile.close();
    }
};

#endif // SETTINGSSTORE_HPP
