#ifndef SETTINGSSTORE_H
#define SETTINGSSTORE_H
#include <QDir>
#include <QStandardPaths>

class SettingsStore : public QObject
{
    Q_OBJECT
public:
    // QStandardPaths::AppConfigLocation appears to not work in a static context, so we have to make it ourselves :(
    static inline const QDir configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QDir::separator() + "QtTube";

    enum FrontPageTab { Home, Trending, Subscriptions, History, None };
    enum PlayerQuality { Auto, HighRes, HD2160, HD1440, HD1080, HD720, Large, Medium, Small, Tiny };
    Q_ENUM(PlayerQuality)

    bool condensedViews = false;
    FrontPageTab frontPageTab = FrontPageTab::Home;
    bool fullSubs = false;
    bool homeShelves = false;
    bool itcCache = true;
    bool playbackTracking = true;
    PlayerQuality preferredQuality = PlayerQuality::Auto;
    int preferredVolume = 100;
    bool returnDislikes = true;
    bool showSBToasts = true;
    bool themedChannels = true;
    QVariantList sponsorBlockCategories;
    bool watchtimeTracking = true;

    static SettingsStore& instance() { static SettingsStore ss; return ss; }
    explicit SettingsStore(QObject* parent = nullptr) : QObject(parent) {}
    void initializeFromSettingsFile();
    void saveToSettingsFile();
};

#endif // SETTINGSSTORE_H
