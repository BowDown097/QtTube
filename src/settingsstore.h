#ifndef SETTINGSSTORE_H
#define SETTINGSSTORE_H
#include <QDir>
#include <QStandardPaths>

class SettingsStore : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SettingsStore)
public:
    // QStandardPaths::AppConfigLocation appears to not work in a static context, so we have to make it ourselves :(
    static inline const QDir configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QDir::separator() + "QtTube";

    enum PlayerQuality { Auto, HighRes, HD2160, HD1440, HD1080, HD720, Large, Medium, Small, Tiny };
    Q_ENUM(PlayerQuality)

    QString appStyle;
    bool condensedViews;
    bool disable60Fps;
    bool disablePlayerInfoPanels;
    bool fullSubs;
    bool h264Only;
    bool homeShelves;
    bool playbackTracking;
    PlayerQuality preferredQuality;
    int preferredVolume;
    bool restoreAnnotations;
    bool returnDislikes;
    bool showSBToasts;
    QStringList sponsorBlockCategories;
    bool themedChannels;
    bool watchtimeTracking;

#ifdef Q_OS_WIN
    bool darkThemeWindows;
#endif

    static SettingsStore& instance() { static SettingsStore ss; return ss; }
    explicit SettingsStore(QObject* parent = nullptr) : QObject(parent) {}

    void initializeFromSettingsFile();
    void saveToSettingsFile();
};

#endif // SETTINGSSTORE_H
