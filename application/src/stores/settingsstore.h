#pragma once
#include "qttube-plugin/components/settings/playersettings.h"
#include "qttube-plugin/objects/video.h"

class QSettings;
class QTimer;

class SettingsStore : public QObject
{
    Q_OBJECT
public:
    QString activePlugin;
    QString appStyle;
    bool autoHideTopBar{};
    bool darkTheme{};
    QString externalPlayerPath;
    int filterLength{};
    bool filterLengthEnabled{};
    QStringList filteredTerms;
    bool imageCaching{};
    QtTubePlugin::PlayerSettings playerSettings;
    bool preferLists{};

    explicit SettingsStore(QObject* parent = nullptr);
    void initialize();
    void save();
    bool strHasFilteredTerm(const QString& str) const;
    bool videoIsFiltered(const QtTubePlugin::Video& video) const;
private:
    QString m_configPath;
    QTimer* m_saveDebounceTimer;

    void readIntoStringList(QSettings& settings, QStringList& list, const QString& prefix, const QString& key);
    void writeStringList(QSettings& settings, const QStringList& list, const QString& prefix, const QString& key);
};
