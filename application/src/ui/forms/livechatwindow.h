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
    explicit LiveChatWindow(QWidget* parent = nullptr);
    ~LiveChatWindow();
private:
    qint64 firstChatItemOffset{};
    qint64 lastChatItemOffset{};
    QTimer* messagesTimer;
    std::any nextData;
    bool populating{};
    QList<QtTube::LiveChatReplayItem> replayItems;
    std::any seekData;
    Ui::LiveChatWindow* ui;
    std::unordered_map<QString, std::any> viewOptions;

    void addChatItemToList(const QtTube::LiveChatItem& item);
    void addNewChatReplayItems(qint64 progress, qint64 previousProgress, bool seeked);
    void createEmojiMenuWidgets();
    void processingEnd();
    void updateChatReplay(qint64 progress, qint64 previousProgress);
    void waitForPopulation();
public slots:
    void initialize(const QtTube::InitialLiveChatData& data, WatchViewPlayer* player);
private slots:
    void chatModeChanged(const QString& name);
    void chatReplayTick(qint64 progress, qint64 previousProgress);
    void chatTick();
    void insertEmoji(const QtTube::Emoji& emoji);
    void processChatData(const QtTube::LiveChat& data);
    void processChatReplayData(qint64 progress, qint64 previousProgress, bool seeked, const QtTube::LiveChatReplay& data);
    void sendMessage();
signals:
    void getLiveChatFinished();
};
