#pragma once
#include "genericstore.h"
#include "qttube-plugin/objects/video.h"

class QSettings;

class SettingsStore : public GenericStore
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
    bool preferLists{};
    bool vaapi{};

    explicit SettingsStore(QObject* parent = nullptr) : GenericStore("settings.ini") {}

    bool strHasFilteredTerm(const QString& str) const;
    bool videoIsFiltered(const QtTubePlugin::Video& video) const;

    void initialize() override;
    void save() override;
private:
    void readIntoStringList(QSettings& settings, QStringList& list, const QString& prefix, const QString& key);
    void writeStringList(QSettings& settings, const QStringList& list, const QString& prefix, const QString& key);
};
