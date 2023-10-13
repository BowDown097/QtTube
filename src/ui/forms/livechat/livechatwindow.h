#ifndef LIVECHATWINDOW_H
#define LIVECHATWINDOW_H
#include "innertube/endpoints/live_chat/getlivechat.h"
#include "innertube/objects/live_chat/livechat.h"
#include <QLabel>
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
    void initialize(const InnertubeObjects::LiveChat& liveChatData);
private:
    QJsonValue actionPanel;
    QString currentContinuation;
    QTimer* messagesTimer;
    int numSentMessages;
    bool timerRunning = false;
    Ui::LiveChatWindow* ui;
private slots:
    void processChatData(const InnertubeEndpoints::GetLiveChat& liveChat);
    void sendMessage();
    void showEmojiMenu();
signals:
    void getLiveChatFinished();
};

#endif // LIVECHATWINDOW_H
