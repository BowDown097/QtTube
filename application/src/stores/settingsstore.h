#pragma once
#include "qttube-plugin/components/configstore.h"
#include "qttube-plugin/components/settings/playersettings.h"
#include "qttube-plugin/objects/video.h"

class QSettings;
class QTimer;

class SettingsStore : public QObject, public QtTubePlugin::ConfigStore
{
    Q_OBJECT
public:
    QString activePlugin;
    QString appStyle;
    bool autoHideTopBar;
    bool darkTheme;
    QString externalPlayerPath;
    int filterLength;
    bool filterLengthEnabled;
    QStringList filteredTerms;
    bool imageCaching;
    bool imageLazyLoading;
    QtTubePlugin::PlayerSettings playerSettings;
    bool preferLists;

    explicit SettingsStore(QObject* parent = nullptr);
    void init() override;
    void save() override;

    bool strHasFilteredTerm(const QString& str) const;
    bool videoIsFiltered(const QtTubePlugin::Video& video) const;
private:
    QTimer* m_saveDebounceTimer;
};
