#pragma once
#include "qttube-plugin/components/replytypes/videodata.h"
#include "qttube-plugin/objects/livechat/livechat.h"
#include "qttube-plugin/objects/livechat/livechatreplay.h"
#include "ui/views/watchviewplayer.h"
#include <QWidget>

namespace Ui {
class LiveChatWindow;
}

class QTimer;

class LiveChatWindow : public QWidget
{
    Q_OBJECT
public:
    explicit LiveChatWindow(PluginEntry* plugin, QWidget* parent = nullptr);
    ~LiveChatWindow();
private:
    QList<QtTubePlugin::LiveChatReplayItem> m_replayItems;
    std::unordered_map<QString, std::any> m_viewOptions;
    QTimer* m_messagesTimer;
    PluginEntry* m_plugin;
    std::any m_nextData;
    std::any m_seekData;
    qint64 m_firstChatItemOffset{};
    qint64 m_lastChatItemOffset{};
    bool m_populating{};
    Ui::LiveChatWindow* ui;

    void addChatItemToList(const QtTubePlugin::LiveChatItem& item);
    void addNewChatReplayItems(qint64 progress, qint64 previousProgress, bool seeked);
    void createEmojiMenuWidgets();
    void processingEnd();
    void updateChatReplay(qint64 progress, qint64 previousProgress);
    void waitForPopulation();
public slots:
    void initialize(const QtTubePlugin::InitialLiveChatData& data, WatchViewPlayer* player);
private slots:
    void chatModeChanged(const QString& name);
    void chatReplayTick(qint64 progress, qint64 previousProgress);
    void chatTick();
    void insertEmoji(const QtTubePlugin::Emoji& emoji);
    void processChatData(const QtTubePlugin::LiveChat& data);
    void processChatReplayData(qint64 progress, qint64 previousProgress, bool seeked, const QtTubePlugin::LiveChatReplay& data);
    void sendMessage();
signals:
    void getLiveChatFinished();
};
