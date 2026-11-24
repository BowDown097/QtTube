#pragma once
#include "plugins/pluginentry.hpp"
#include <qttube-plugin/components/player/player.h>
#include <qttube-plugin/components/replytypes/videodata.h>

class WatchViewPlayer : public QObject
{
    Q_OBJECT
public:
    enum class ScaleMode { Unset, NoScale, Scaled };

    WatchViewPlayer(QWidget* watchView, PluginEntry* plugin, const QSize& maxSize);
    void calcAndSetSize(const QSize& maxSize);
    void seek(int progress);
    QWidget* widget();

    void tryPlayDirect(const QList<QtTubePlugin::VideoSource>& sources, int progress = 0);
    void tryPlayImmediate(const QString& videoId, int progress = 0);

    QSize size() const { return m_size; }
    ScaleMode scaleMode() const { return m_scaleMode; }
private:
    QtTubePlugin::Player* m_player{};
    PluginEntry* m_plugin;
    ScaleMode m_scaleMode;
    QSize m_size;
private slots:
    void copyToClipboard(const QString& text);
    void newState(QtTubePlugin::Player::PlayerState state);
    void switchVideo(PluginEntry* plugin, const QString& videoId);
signals:
    void progressChanged(qint64 progress, qint64 previousProgress);
    void scaleModeChanged(WatchViewPlayer::ScaleMode scaleMode);
};
