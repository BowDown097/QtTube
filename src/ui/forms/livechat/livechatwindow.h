#ifndef LIVECHATWINDOW_H
#define LIVECHATWINDOW_H
#include "ui/views/watchviewplayer.h"
#include <QJsonArray>
#include <QWidget>

namespace Ui {
class LiveChatWindow;
}

namespace InnertubeEndpoints
{
class GetLiveChat;
class GetLiveChatReplay;
}

namespace InnertubeObjects { class LiveChat; }

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
    void initialize(const InnertubeObjects::LiveChat& liveChatData, WatchViewPlayer* player);
private:
    QJsonValue actionPanel;
    QString currentContinuation;
    double firstChatItemOffset{};
    double lastChatItemOffset{};
    QTimer* messagesTimer;
    int numSentMessages{};
    bool populating{};
    QJsonArray replayActions;
    QString seekContinuation;

    TubeLabel* emojiMenuLabel;
    Ui::LiveChatWindow* ui;

    void addChatItemToList(const QJsonValue& item);
    void addNewChatReplayItems(double progress, double previousProgress, bool seeked);
    void processingEnd();
    void updateChatReplay(double progress, double previousProgress);
private slots:
    void chatReplayTick(double progress, double previousProgress);
    void chatTick();
    void processChatData(const InnertubeEndpoints::GetLiveChat& liveChat);
    void processChatReplayData(const InnertubeEndpoints::GetLiveChatReplay& replay,
                               double progress, double previousProgress, bool seeked);
    void sendMessage();
    void showEmojiMenu();
signals:
    void getLiveChatFinished();
};

#endif // LIVECHATWINDOW_H
