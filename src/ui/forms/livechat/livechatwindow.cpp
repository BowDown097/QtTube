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

LiveChatWindow::LiveChatWindow(QWidget* parent) : QWidget(parent), ui(new Ui::LiveChatWindow)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    TubeLabel* emojiMenuLabel = new TubeLabel(this);
    emojiMenuLabel->setClickable(true, false);
    emojiMenuLabel->setPixmap(UIUtils::pixmapThemed("emoji", true, QSize(ui->messageBox->height() - 8, ui->messageBox->height() - 8)));
    ui->horizontalLayout->insertWidget(0, emojiMenuLabel);

    connect(emojiMenuLabel, &TubeLabel::clicked, this, &LiveChatWindow::showEmojiMenu);
    connect(ui->messageBox, &QLineEdit::returnPressed, this, &LiveChatWindow::sendMessage);
    connect(ui->sendButton, &QPushButton::pressed, this, &LiveChatWindow::sendMessage);
}

void LiveChatWindow::initialize(const InnertubeObjects::LiveChat& liveChatData)
{
    currentContinuation = liveChatData.continuations[0];

    messagesTimer = new QTimer(this);
    messagesTimer->start(1000);
    connect(messagesTimer, &QTimer::timeout, this, [this] {
        if (timerRunning) return;
        InnertubeReply* reply = InnerTube::instance().get<InnertubeEndpoints::GetLiveChat>(currentContinuation);
        connect(reply, qOverload<const InnertubeEndpoints::GetLiveChat&>(&InnertubeReply::finished), this, &LiveChatWindow::processChatData);
    });
}

void LiveChatWindow::processChatData(const InnertubeEndpoints::GetLiveChat& liveChat)
{
    timerRunning = true;

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

    const QJsonArray actions = liveChat.liveChatContinuation["actions"].toArray();
    for (const QJsonValue& v : actions)
    {
        if (!v["addChatItemAction"].isObject())
            continue;

        QJsonValue item = v["addChatItemAction"]["item"];
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

    if (ui->listWidget->count() > 200)
    {
        for (int i = 0; i < ui->listWidget->count() - 200; i++)
        {
            QListWidgetItem* item = ui->listWidget->takeItem(i);
            delete item;
        }
    }

    ui->listWidget->scrollToBottom();
    currentContinuation = liveChat.liveChatContinuation["continuations"][0]["invalidationContinuationData"]["continuation"].toString();
    timerRunning = false;
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
    QJsonArray textSegments = ytemoji::produceRichText(ytemoji::emojize(ui->messageBox->text().trimmed()));

    InnerTube::instance().sendMessage(textSegments, clientMessageId, params);
    ui->messageBox->clear();
}

void LiveChatWindow::showEmojiMenu()
{
    EmojiMenu* emojiMenu = new EmojiMenu;
    emojiMenu->show();
    connect(emojiMenu, &EmojiMenu::emojiClicked, this, [this](const QString& emoji) {
        ui->messageBox->insert(" " + emoji);
    });
}

LiveChatWindow::~LiveChatWindow()
{
    // avoid use-after-free when closing
    QEventLoop loop;
    connect(this, &LiveChatWindow::getLiveChatFinished, &loop, &QEventLoop::quit);
    loop.exec();

    messagesTimer->deleteLater();
    delete ui;
}
