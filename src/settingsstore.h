#ifndef SETTINGSSTORE_H
#define SETTINGSSTORE_H
#include <QDir>
#include <QStandardPaths>

class SettingsStore
{
public:
    // QStandardPaths::AppConfigLocation appears to not work in a static context, so we have to make it ourselves :(
    enum FrontPageTab { Home, Trending, Subscriptions, History, None };
    static inline const QDir configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QDir::separator() + "QtTube";
    bool condensedViews = false;
    FrontPageTab frontPageTab = FrontPageTab::Home;
    bool fullSubs = false;
    bool homeShelves = false;
    bool itcCache = true;
    bool playbackTracking = true;
    int preferredVolume = 100;
    bool returnDislikes = true;
    bool showSBToasts = true;
    QVariantList sponsorBlockCategories;
    bool watchtimeTracking = true;

    static SettingsStore& instance() { static SettingsStore ss; return ss; }
    void initializeFromSettingsFile();
    void saveToSettingsFile();
};

#endif // SETTINGSSTORE_H
