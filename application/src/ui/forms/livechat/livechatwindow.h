#pragma once
#include "ui/views/watchviewplayer.h"
#include <QJsonArray>
#include <QWidget>

namespace Ui {
class LiveChatWindow;
}

namespace InnertubeEndpoints
{
struct GetLiveChat;
struct GetLiveChatReplay;
}

class QJsonArray;
class QTimer;
class TubeLabel;

class LiveChatWindow : public QWidget
{
    Q_OBJECT
public:
    explicit LiveChatWindow(QWidget* parent = nullptr);
    ~LiveChatWindow();
public slots:
    void initialize(const QString& continuation, bool isReplay, WatchViewPlayer* player);
private:
    QJsonValue actionPanel;
    QString currentContinuation;
    double firstChatItemOffset{};
    double lastChatItemOffset{};
    QString liveChatReloadContinuation;
    QTimer* messagesTimer;
    int numSentMessages{};
    bool populating{};
    QJsonArray replayActions;
    QString seekContinuation;
    QString topChatReloadContinuation;

    TubeLabel* emojiMenuLabel;
    Ui::LiveChatWindow* ui;

    void addChatItemToList(const QJsonValue& item);
    void addNewChatReplayItems(double progress, double previousProgress, bool seeked);
    void processingEnd();
    void updateChatReplay(double progress, double previousProgress);
private slots:
    void chatModeIndexChanged(int index);
    void chatReplayTick(double progress, double previousProgress);
    void chatTick();
    void insertEmoji(const QString& emoji);
    void processChatData(const InnertubeEndpoints::GetLiveChat& liveChat);
    void processChatReplayData(double progress, double previousProgress, bool seeked,
                               const InnertubeEndpoints::GetLiveChatReplay& replay);
    void sendMessage();
    void showEmojiMenu();
signals:
    void getLiveChatFinished();
};
