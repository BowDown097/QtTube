#ifndef LIVECHATWINDOW_H
#define LIVECHATWINDOW_H
#include "innertube/endpoints/live_chat/getlivechat.h"
#include "innertube/endpoints/live_chat/getlivechatreplay.h"
#include "innertube/objects/live_chat/livechat.h"
#include "ui/views/watchviewplayer.h"
#include "ui/widgets/labels/tubelabel.h"
#include <QJsonArray>
#include <QTimer>

namespace Ui {
class LiveChatWindow;
}

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
