#pragma once
#include <qttube-plugin/components/player/webplayer.h>
#include <quickjs++/value.h>

class ScriptPluginWebPlayer : public QtTubePlugin::WebPlayer
{
    friend class ScriptPluginEntry;
public:
    ScriptPluginWebPlayer(
        QtTubePlugin::PlayerSettings* settings, const qjs::value& player_v, QWidget* parent);
    void play(const QString& videoId, int progress) override;
    void seek(int progress) override;
private:
    qjs::value player_v;
    std::unordered_map<QString, std::unique_ptr<QObject>> webChannelObjects;

    qjs::value makeSelfObject() const;
};
