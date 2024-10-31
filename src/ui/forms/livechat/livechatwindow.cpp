#include "livechatwindow.h"
#include "ui_livechatwindow.h"
#include "innertube.h"
#include "ui/forms/emojimenu.h"
#include "ui/widgets/labels/tubelabel.h"
#include "utils/uiutils.h"
#include "ytemoji.h"
#include "giftredemptionmessage.h"
#include "paidmessage.h"
#include "specialmessage.h"
#include "textmessage.h"
#include <QTimer>

LiveChatWindow::LiveChatWindow(QWidget* parent)
    : QWidget(parent), emojiMenuLabel(new TubeLabel(this)), messagesTimer(new QTimer(this)), ui(new Ui::LiveChatWindow)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    emojiMenuLabel->setClickable(true);
    emojiMenuLabel->setPixmap(UIUtils::pixmapThemed("emoji", true, QSize(ui->messageBox->height() - 8, ui->messageBox->height() - 8)));
    ui->horizontalLayout->insertWidget(0, emojiMenuLabel);

    connect(emojiMenuLabel, &TubeLabel::clicked, this, &LiveChatWindow::showEmojiMenu);
    connect(ui->chatModeSwitcher, qOverload<int>(&QComboBox::currentIndexChanged), this, &LiveChatWindow::chatModeIndexChanged);
    connect(ui->messageBox, &QLineEdit::returnPressed, this, &LiveChatWindow::sendMessage);
    connect(ui->sendButton, &QPushButton::pressed, this, &LiveChatWindow::sendMessage);
}

void LiveChatWindow::addChatItemToList(const QJsonValue& item)
{
    if (item["liveChatTextMessageRenderer"].isObject()) [[likely]]
        UIUtils::addWidgetToList(ui->listWidget, new TextMessage(item["liveChatTextMessageRenderer"], this));
    else if (item["liveChatMembershipItemRenderer"].isObject())
        UIUtils::addWidgetToList(ui->listWidget, new SpecialMessage(item["liveChatMembershipItemRenderer"], this, "authorName", "headerSubtext", false, "#0f9d58"));
    else if (item["liveChatModeChangeMessageRenderer"].isObject())
        UIUtils::addWidgetToList(ui->listWidget, new SpecialMessage(item["liveChatModeChangeMessageRenderer"], this));
    else if (item["liveChatPaidMessageRenderer"].isObject())
        UIUtils::addWidgetToList(ui->listWidget, new PaidMessage(item["liveChatPaidMessageRenderer"], this));
    else if (item["liveChatSponsorshipsGiftRedemptionAnnouncementRenderer"].isObject())
        UIUtils::addWidgetToList(ui->listWidget, new GiftRedemptionMessage(item["liveChatSponsorshipsGiftRedemptionAnnouncementRenderer"], this));
    else if (item["liveChatViewerEngagementMessageRenderer"].isObject())
        UIUtils::addWidgetToList(ui->listWidget, new SpecialMessage(item["liveChatViewerEngagementMessageRenderer"], this, "text", "message", false));
}

void LiveChatWindow::addNewChatReplayItems(double progress, double previousProgress, bool seeked)
{
    for (const QJsonValue& v : std::as_const(replayActions))
    {
        if (!v["replayChatItemAction"]["actions"].isArray())
            continue;

        double videoOffsetTimeSec = v["replayChatItemAction"]["videoOffsetTimeMsec"].toString().toDouble() / 1000;
        if ((!seeked && videoOffsetTimeSec < previousProgress) || videoOffsetTimeSec > progress)
            continue;

        const QJsonArray itemActions = v["replayChatItemAction"]["actions"].toArray();
        for (const QJsonValue& v2 : itemActions)
            if (v2["addChatItemAction"]["item"].isObject())
                addChatItemToList(v2["addChatItemAction"]["item"]);
    }
}

void LiveChatWindow::chatModeIndexChanged(int index)
{
    QString continuation = index == 0 ? topChatReloadContinuation : liveChatReloadContinuation;
    if (continuation.isEmpty())
    {
        qDebug() << "Failed to change chat mode: continuation is missing.";
        return;
    }

    if (populating)
    {
        QEventLoop loop;
        connect(this, &LiveChatWindow::getLiveChatFinished, &loop, &QEventLoop::quit);
        loop.exec();
    }

    currentContinuation = continuation;
    ui->listWidget->clear();
}

void LiveChatWindow::chatReplayTick(double progress, double previousProgress)
{
    if (populating)
        return;

    populating = true;
    QString playerOffsetMs = QString::number(int(progress * 1000));
    int progDiff = progress - previousProgress;

    if (previousProgress > 0 && (progDiff <= -1 || progDiff > 1))
    {
        ui->listWidget->clear();
        auto reply = InnerTube::instance()->get<InnertubeEndpoints::GetLiveChatReplay>(seekContinuation, playerOffsetMs);
        connect(reply, &InnertubeReply<InnertubeEndpoints::GetLiveChatReplay>::finished, this,
                std::bind_front(&LiveChatWindow::processChatReplayData, this, progress, previousProgress, true));
    }
    else if (progress < firstChatItemOffset || progress > lastChatItemOffset)
    {
        auto reply = InnerTube::instance()->get<InnertubeEndpoints::GetLiveChatReplay>(currentContinuation, playerOffsetMs);
        connect(reply, &InnertubeReply<InnertubeEndpoints::GetLiveChatReplay>::finished, this,
                std::bind_front(&LiveChatWindow::processChatReplayData, this, progress, previousProgress, false));
    }
    else
    {
        updateChatReplay(progress, previousProgress);
    }
}

void LiveChatWindow::chatTick()
{
    if (populating)
        return;

    populating = true;
    auto reply = InnerTube::instance()->get<InnertubeEndpoints::GetLiveChat>(currentContinuation);
    connect(reply, &InnertubeReply<InnertubeEndpoints::GetLiveChat>::finished, this, &LiveChatWindow::processChatData);
}

void LiveChatWindow::initialize(const InnertubeObjects::LiveChat& liveChatData, WatchViewPlayer* player)
{
    currentContinuation = liveChatData.continuations[0];

    if (liveChatData.isReplay)
    {
        emojiMenuLabel->hide();
        ui->chatModeSwitcher->hide();
        ui->messageBox->hide();
        ui->sendButton->hide();
        connect(player, &WatchViewPlayer::progressChanged, this, std::bind_front(&LiveChatWindow::chatReplayTick, this));
    }
    else
    {
        messagesTimer->start(1000);
        connect(messagesTimer, &QTimer::timeout, this, &LiveChatWindow::chatTick);
    }
}

void LiveChatWindow::insertEmoji(const QString& emoji)
{
    if (QString msg = ui->messageBox->text(); !msg.isEmpty() && ui->messageBox->cursorPosition() == msg.size())
        ui->messageBox->insert(" " + emoji);
    else
        ui->messageBox->insert(emoji);
}

void LiveChatWindow::processChatData(const InnertubeEndpoints::GetLiveChat& liveChat)
{
    // check if user can chat
    if (liveChat.liveChatContinuation["actionPanel"].isObject())
    {
        actionPanel = liveChat.liveChatContinuation["actionPanel"];
        if (actionPanel["liveChatRestrictedParticipationRenderer"].isObject())
        {
            InnertubeObjects::InnertubeString message(actionPanel["liveChatRestrictedParticipationRenderer"]["message"]);
            ui->messageBox->setEnabled(false);
            ui->messageBox->setPlaceholderText(message.text);
            ui->sendButton->setEnabled(false);
        }
        else if (ui->messageBox->placeholderText() != "Say something...")
        {
            ui->messageBox->setEnabled(true);
            ui->messageBox->setPlaceholderText("Say something...");
            ui->sendButton->setEnabled(true);
        }
    }

    // get top and live chat continuations
    if (liveChat.liveChatContinuation["header"]["liveChatHeaderRenderer"]["viewSelector"].isObject())
    {
        QJsonValue sortFilter = liveChat.liveChatContinuation["header"]["liveChatHeaderRenderer"]["viewSelector"]
                                                             ["sortFilterSubMenuRenderer"]["subMenuItems"];
        topChatReloadContinuation = sortFilter[0]["continuation"]["reloadContinuationData"]["continuation"].toString();
        liveChatReloadContinuation = sortFilter[1]["continuation"]["reloadContinuationData"]["continuation"].toString();
    }

    const QJsonArray actions = liveChat.liveChatContinuation["actions"].toArray();
    for (const QJsonValue& v : actions)
        if (v["addChatItemAction"]["item"].isObject())
            addChatItemToList(v["addChatItemAction"]["item"]);

    QString continuation = liveChat.liveChatContinuation["continuations"][0]["invalidationContinuationData"]["continuation"].toString();
    if (!continuation.isEmpty())
        currentContinuation = continuation;
    else if (liveChat.liveChatContinuation["continuations"][0]["reloadContinuationData"].isObject()) // should be true if live stream has finished
        messagesTimer->stop();

    processingEnd();
}

void LiveChatWindow::processChatReplayData(double progress, double previousProgress, bool seeked,
                                           const InnertubeEndpoints::GetLiveChatReplay& replay)
{
    replayActions = replay.liveChatContinuation["actions"].toArray();
    addNewChatReplayItems(progress, previousProgress, seeked);

    QJsonValue liveChatReplayContinuationData = replay.liveChatContinuation["continuations"][0]["liveChatReplayContinuationData"];
    if (liveChatReplayContinuationData["continuation"].isString())
        currentContinuation = liveChatReplayContinuationData["continuation"].toString();
    if (!liveChatReplayContinuationData["timeUntilLastMessageMsec"].isUndefined())
        messagesTimer->setInterval(liveChatReplayContinuationData["timeUntilLastMessageMsec"].toInt());

    firstChatItemOffset = replayActions.first()["replayChatItemAction"]["videoOffsetTimeMsec"].toString().toDouble() / 1000;
    lastChatItemOffset = replayActions.last()["replayChatItemAction"]["videoOffsetTimeMsec"].toString().toDouble() / 1000;
    seekContinuation = replay.liveChatContinuation["continuations"][1]["playerSeekContinuationData"]["continuation"].toString();

    processingEnd();
}

void LiveChatWindow::processingEnd()
{
    if (ui->listWidget->count() > 250)
        for (int i = 0; i < ui->listWidget->count() - 250; i++)
            delete ui->listWidget->takeItem(i);

    ui->listWidget->scrollToBottom();
    populating = false;
    emit getLiveChatFinished();
}

void LiveChatWindow::sendMessage()
{
    if (ui->messageBox->text().trimmed().isEmpty())
        return;

    QJsonValue sendEndpoint = actionPanel["liveChatMessageInputRenderer"]["sendButton"]["buttonRenderer"]
                                         ["serviceEndpoint"]["sendLiveChatMessageEndpoint"];

    QString clientMessageId = sendEndpoint["clientIdPrefix"].toString() + QString::number(numSentMessages++);
    QString params = sendEndpoint["params"].toString();
    QJsonArray textSegments = ytemoji::instance()->produceRichText(ytemoji::instance()->emojize(ui->messageBox->text().trimmed()));

    InnerTube::instance()->sendMessage(textSegments, clientMessageId, params);
    ui->messageBox->clear();
}

void LiveChatWindow::showEmojiMenu()
{
    EmojiMenu* emojiMenu = new EmojiMenu;
    emojiMenu->show();
    connect(emojiMenu, &EmojiMenu::emojiClicked, this, &LiveChatWindow::insertEmoji);
}

void LiveChatWindow::updateChatReplay(double progress, double previousProgress)
{
    addNewChatReplayItems(progress, previousProgress, false);
    processingEnd();
}

LiveChatWindow::~LiveChatWindow()
{
    // avoid use-after-free when closing
    if (populating)
    {
        QEventLoop loop;
        connect(this, &LiveChatWindow::getLiveChatFinished, &loop, &QEventLoop::quit);
        loop.exec();
    }

    messagesTimer->deleteLater();
    delete ui;
}
