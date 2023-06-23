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
    bool condensedViews = false;
    bool darkTheme = false;
    bool disable60Fps = false;
    bool disablePlayerInfoPanels = false;
    bool fullSubs = false;
    bool h264Only = false;
    bool homeShelves = false;
    bool playbackTracking = true;
    PlayerQuality preferredQuality = PlayerQuality::Auto;
    int preferredVolume = 100;
    bool restoreAnnotations = false;
    bool returnDislikes = true;
    bool showSBToasts = true;
    QStringList sponsorBlockCategories;
    bool themedChannels = false;
    bool watchtimeTracking = true;

    static SettingsStore& instance() { static SettingsStore ss; return ss; }
    explicit SettingsStore(QObject* parent = nullptr) : QObject(parent) {}

    void initializeFromSettingsFile();
    void saveToSettingsFile();
};

#endif // SETTINGSSTORE_H
