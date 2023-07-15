#ifndef SETTINGSSTORE_H
#define SETTINGSSTORE_H
#include <mutex>
#include <QDir>
#include <QStandardPaths>

class SettingsStore : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString appStyle MEMBER appStyle NOTIFY appStyleChanged)
    Q_PROPERTY(bool condensedViews MEMBER condensedViews NOTIFY condensedViewsChanged)
    Q_PROPERTY(bool darkTheme MEMBER darkTheme NOTIFY darkThemeChanged)
    Q_PROPERTY(bool disable60Fps MEMBER disable60Fps NOTIFY disable60FpsChanged)
    Q_PROPERTY(bool disablePlayerInfoPanels MEMBER disablePlayerInfoPanels NOTIFY disablePlayerInfoPanelsChanged)
    Q_PROPERTY(bool fullSubs MEMBER fullSubs NOTIFY fullSubsChanged)
    Q_PROPERTY(bool h264Only MEMBER h264Only NOTIFY h264OnlyChanged)
    Q_PROPERTY(bool homeShelves MEMBER homeShelves NOTIFY homeShelvesChanged)
    Q_PROPERTY(bool playbackTracking MEMBER playbackTracking NOTIFY playbackTrackingChanged)
    Q_PROPERTY(PlayerQuality preferredQuality MEMBER preferredQuality NOTIFY preferredQualityChanged)
    Q_PROPERTY(int preferredVolume MEMBER preferredVolume NOTIFY preferredVolumeChanged)
    Q_PROPERTY(bool restoreAnnotations MEMBER restoreAnnotations NOTIFY restoreAnnotationsChanged)
    Q_PROPERTY(bool returnDislikes MEMBER returnDislikes NOTIFY returnDislikesChanged)
    Q_PROPERTY(bool showSBToasts MEMBER showSBToasts NOTIFY showSBToastsChanged)
    Q_PROPERTY(QStringList sponsorBlockCategories MEMBER sponsorBlockCategories NOTIFY sponsorBlockCategoriesChanged)
    Q_PROPERTY(bool themedChannels MEMBER themedChannels NOTIFY themedChannelsChanged)
    Q_PROPERTY(bool watchtimeTracking MEMBER watchtimeTracking NOTIFY watchtimeTrackingChanged)
public:
    // QStandardPaths::AppConfigLocation appears to not work in a static context, so we have to make it ourselves :(
    static inline const QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QDir::separator() + "QtTube"
                                            + QDir::separator() + "settings.ini";

    enum class PlayerQuality { Auto, HighRes, HD2160, HD1440, HD1080, HD720, Large, Medium, Small, Tiny };
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

    static SettingsStore* instance();
    explicit SettingsStore(QObject* parent = nullptr) : QObject(parent) {}

    void initializeFromSettingsFile();
    void saveToSettingsFile();
private:
    static inline SettingsStore* m_instance;
    static inline std::once_flag m_onceFlag;
signals:
    void appStyleChanged(const QString&);
    void condensedViewsChanged(bool);
    void darkThemeChanged(bool);
    void disable60FpsChanged(bool);
    void disablePlayerInfoPanelsChanged(bool);
    void fullSubsChanged(bool);
    void h264OnlyChanged(bool);
    void homeShelvesChanged(bool);
    void playbackTrackingChanged(bool);
    void preferredQualityChanged(SettingsStore::PlayerQuality);
    void preferredVolumeChanged(int);
    void restoreAnnotationsChanged(bool);
    void returnDislikesChanged(bool);
    void showSBToastsChanged(bool);
    void sponsorBlockCategoriesChanged(const QStringList&);
    void themedChannelsChanged(bool);
    void watchtimeTrackingChanged(bool);
};

#endif // SETTINGSSTORE_H
