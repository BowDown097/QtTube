#pragma once
#include "qttube-plugin/components/replytypes.h"
#include "qttube-plugin/objects/emoji.h"
#include "ui/views/watchviewplayer.h"
#include <QWidget>

namespace Ui {
class LiveChatWindow;
}

class EmojiMenu;
class QTimer;
class TubeLabel;

class LiveChatWindow : public QWidget
{
    Q_OBJECT
public:
    explicit LiveChatWindow(QWidget* parent = nullptr);
    ~LiveChatWindow();
public slots:
    void initialize(const QtTube::InitialLiveChatData& data, WatchViewPlayer* player);
private:
    EmojiMenu* emojiMenu;
    TubeLabel* emojiMenuLabel;
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
    void addNewChatReplayItems(double progress, double previousProgress, bool seeked);
    void processingEnd();
    void updateChatReplay(double progress, double previousProgress);
    void waitForPopulation();
private slots:
    void chatModeChanged(const QString& name);
    void chatReplayTick(double progress, double previousProgress);
    void chatTick();
    void insertEmoji(const QtTube::Emoji& emoji);
    void processChatData(const QtTube::LiveChat& data);
    void processChatReplayData(double progress, double previousProgress, bool seeked, const QtTube::LiveChatReplay& data);
    void sendMessage();
signals:
    void getLiveChatFinished();
};
