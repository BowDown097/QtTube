#ifndef SETTINGSSTORE_H
#define SETTINGSSTORE_H
#include <QDir>
#include <QSettings>
#include <QStandardPaths>

class SettingsStore : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool blockAds MEMBER blockAds NOTIFY blockAdsChanged)
    Q_PROPERTY(bool disablePlayerInfoPanels MEMBER disablePlayerInfoPanels NOTIFY disablePlayerInfoPanelsChanged)
    Q_PROPERTY(PlayerQuality preferredQuality MEMBER preferredQuality NOTIFY preferredQualityChanged)
    Q_PROPERTY(int preferredVolume MEMBER preferredVolume NOTIFY preferredVolumeChanged)
    Q_PROPERTY(bool restoreAnnotations MEMBER restoreAnnotations NOTIFY restoreAnnotationsChanged)
    Q_PROPERTY(QStringList sponsorBlockCategories MEMBER sponsorBlockCategories NOTIFY sponsorBlockCategoriesChanged)
public:
    // QStandardPaths::AppConfigLocation appears to not work in a static context, so we have to make it ourselves :(
    static inline const QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
                                           + QDir::separator() + "QtTube"
                                           + QDir::separator() + "settings.ini";

    enum class PlayerQuality { Auto, HighRes, HD2160, HD1440, HD1080, HD720, Large, Medium, Small, Tiny };
    Q_ENUM(PlayerQuality)

    QString appStyle;
    bool blockAds;
    bool condensedViews;
    bool darkTheme;
    bool deArrow;
    bool deArrowThumbs;
    bool deArrowTitles;
    bool disable60Fps;
    bool disablePlayerInfoPanels;
    int filterLength;
    bool filterLengthEnabled;
    QStringList filteredChannels;
    QStringList filteredTerms;
    bool fullSubs;
    bool h264Only;
    bool hideShorts;
    bool hideStreams;
    bool playbackTracking;
    bool preferLists;
    PlayerQuality preferredQuality;
    int preferredVolume;
    bool restoreAnnotations;
    bool returnDislikes;
    bool showSBToasts;
    QStringList sponsorBlockCategories;
    bool vaapi;
    bool watchtimeTracking;

    explicit SettingsStore(QObject* parent = nullptr) : QObject(parent) {}
    bool channelIsFiltered(const QString& channelId) const;
    void initialize();
    void save();
    bool strHasFilteredTerm(const QString& str) const;
private:
    void readIntoStringList(QSettings& settings, QStringList& list, const QString& prefix, const QString& key);
    void writeStringList(QSettings& settings, const QStringList& list, const QString& prefix, const QString& key);
signals:
    void blockAdsChanged(bool);
    void disablePlayerInfoPanelsChanged(bool);
    void preferredQualityChanged(SettingsStore::PlayerQuality);
    void preferredVolumeChanged(int);
    void restoreAnnotationsChanged(bool);
    void sponsorBlockCategoriesChanged(const QStringList&);
};

#endif // SETTINGSSTORE_H
