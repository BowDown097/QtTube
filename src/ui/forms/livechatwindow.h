#ifndef LIVECHATWINDOW_H
#define LIVECHATWINDOW_H
#include "innertube/endpoints/live_chat/getlivechat.h"
#include "innertube/objects/live_chat/livechat.h"
#include <QTimer>
#include <QWidget>

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
    void addSpecialMessage(const QJsonValue& messageRenderer, const QString& headerKey = "text", const QString& subtextKey = "subtext",
                           bool subtextItalic = true, const QString& background = "black");

    QJsonValue actionPanel;
    QString currentContinuation;
    bool firstTimerRun = true;
    QTimer* messagesTimer;
    int numSentMessages;
    Ui::LiveChatWindow* ui;
private slots:
    void processChatData(const InnertubeEndpoints::GetLiveChat& liveChat);
    void sendMessage();
};

#endif // LIVECHATWINDOW_H
