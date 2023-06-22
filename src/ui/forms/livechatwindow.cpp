#include "livechatwindow.h"
#include "ui_livechatwindow.h"
#include "emoji.h"
#include "http.h"
#include "innertube.h"
#include "ui/forms/emojimenu.h"
#include "ui/widgets/labels/tubelabel.h"
#include "ui/uiutilities.h"
#include <QScrollBar>
#include <QVBoxLayout>

LiveChatWindow::LiveChatWindow(QWidget* parent) : QWidget(parent), ui(new Ui::LiveChatWindow)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    TubeLabel* emojiMenuLabel = new TubeLabel(this);
    emojiMenuLabel->setClickable(true, false);
    emojiMenuLabel->setPixmap(UIUtilities::icon("emoji", true, QSize(ui->messageBox->height() - 8, ui->messageBox->height() - 8)));
    ui->horizontalLayout->insertWidget(0, emojiMenuLabel);

    connect(emojiMenuLabel, &TubeLabel::clicked, this, &LiveChatWindow::showEmojiMenu);
    connect(ui->messageBox, &QLineEdit::returnPressed, this, &LiveChatWindow::sendMessage);
    connect(ui->sendButton, &QPushButton::pressed, this, &LiveChatWindow::sendMessage);
}

void LiveChatWindow::initialize(const InnertubeObjects::LiveChat& liveChatData)
{
    currentContinuation = liveChatData.continuations[0];

    messagesTimer = new QTimer(this);
    connect(messagesTimer, &QTimer::timeout, this, [this] {
        if (timerRunning)
            return;
        InnertubeReply* reply = InnerTube::instance().get<InnertubeEndpoints::GetLiveChat>(currentContinuation);
        connect(reply, qOverload<const InnertubeEndpoints::GetLiveChat&>(&InnertubeReply::finished), this, &LiveChatWindow::processChatData);
    });
    messagesTimer->start(1000);
}

void LiveChatWindow::addSpecialMessage(const QJsonValue& messageRenderer, const QString& headerKey, const QString& subtextKey,
                                       bool subtextItalic, const QString& background)
{
    InnertubeObjects::InnertubeString header(messageRenderer[headerKey]);
    InnertubeObjects::InnertubeString subtext(messageRenderer[subtextKey]);

    QWidget* messageWidget = new QWidget(this);
    messageWidget->setAutoFillBackground(true);
    messageWidget->setStyleSheet(QStringLiteral("background: %1; border: 1px solid transparent; border-radius: 4px")
                                     .arg(background));

    QVBoxLayout* messageLayout = new QVBoxLayout(messageWidget);
    messageLayout->setContentsMargins(0, 0, 0, 0);

    if (!header.text.isEmpty())
    {
        QLabel* headerLabel = new QLabel(header.text, this);
        headerLabel->setAlignment(Qt::AlignCenter);
        headerLabel->setFont(QFont(qApp->font().toString(), -1, QFont::Bold));
        headerLabel->setWordWrap(true);
        messageLayout->addWidget(headerLabel);
    }

    QLabel* subtextLabel = new QLabel(subtext.text, this);
    subtextLabel->setAlignment(Qt::AlignCenter);
    subtextLabel->setFont(QFont(qApp->font().toString(), -1, -1, subtextItalic));
    subtextLabel->setWordWrap(true);
    messageLayout->addWidget(subtextLabel);

    UIUtilities::addWidgetToList(ui->listWidget, messageWidget);
}

void LiveChatWindow::processChatData(const InnertubeEndpoints::GetLiveChat& liveChat)
{
    timerRunning = true;

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
        if (item["liveChatTextMessageRenderer"].isObject())
        {
            QJsonValue liveChatTextMessage = item["liveChatTextMessageRenderer"];

            QWidget* messageWidget = new QWidget(this);

            QHBoxLayout* messageLayout = new QHBoxLayout(messageWidget);
            messageLayout->setContentsMargins(0, 0, 0, 0);
            messageLayout->setSpacing(0);

            QLabel* authorIcon = new QLabel(this);
            authorIcon->setFixedSize(38, 32);
            messageLayout->addWidget(authorIcon);

            HttpReply* iconReply = Http::instance().get(liveChatTextMessage["authorPhoto"]["thumbnails"][0]["url"].toString());
            connect(iconReply, &HttpReply::finished, this, std::bind(&LiveChatWindow::setAuthorIcon, this, std::placeholders::_1, authorIcon));

            QVBoxLayout* contentLayout = new QVBoxLayout;
            contentLayout->setContentsMargins(0, 0, 0, 0);
            contentLayout->setSpacing(0);

            QLabel* authorLabel = new QLabel(liveChatTextMessage["authorName"]["simpleText"].toString(), this);
            authorLabel->setFixedWidth(ui->listWidget->width() - 50);
            authorLabel->setStyleSheet(liveChatTextMessage.toObject().contains("authorBadges")
                                       ? "font-weight: bold; color: #2ba640"
                                       : "font-weight: bold");
            contentLayout->addWidget(authorLabel);

            QLabel* messageLabel = new QLabel(InnertubeObjects::InnertubeString(liveChatTextMessage["message"]).text, this);
            messageLabel->setFixedWidth(ui->listWidget->width() - 50);
            messageLabel->setTextFormat(Qt::RichText);
            messageLabel->setWordWrap(true);

            QString messageText;
            const QJsonArray runs = liveChatTextMessage["message"]["runs"].toArray();
            for (const QJsonValue& v : runs)
            {
                if (v["emoji"].isObject())
                {
                    HttpReply* emojiReply = Http::instance().get(v["emoji"]["image"]["thumbnails"][0]["url"].toString());

                    QEventLoop loop;
                    connect(emojiReply, &HttpReply::finished, &loop, &QEventLoop::quit);
                    loop.exec();

                    messageText += QStringLiteral("<img src='data:%1;base64,%2' width='20' height='20'>")
                                       .arg(emojiReply->header("content-type"), emojiReply->body().toBase64());
                    emojiReply->deleteLater();
                }
                else if (v["text"].isString())
                {
                    messageText += v["text"].toString();
                }
            }

            messageLabel->setText(messageText);
            contentLayout->addWidget(messageLabel);

            contentLayout->addStretch();
            messageLayout->addLayout(contentLayout);

            UIUtilities::addWidgetToList(ui->listWidget, messageWidget);
        }
        else if (item["liveChatMembershipItemRenderer"].isObject())
        {
            addSpecialMessage(item["liveChatMembershipItemRenderer"], "authorName", "headerSubtext", false, "#0f9d58");
        }
        else if (item["liveChatModeChangeMessageRenderer"].isObject())
        {
            addSpecialMessage(item["liveChatModeChangeMessageRenderer"]);
        }
        else if (item["liveChatPaidMessageRenderer"].isObject())
        {
            QJsonValue paidMessage = item["liveChatPaidMessageRenderer"];

            QWidget* messageWidget = new QWidget(this);

            QVBoxLayout* messageLayout = new QVBoxLayout(messageWidget);
            messageLayout->setContentsMargins(0, 0, 0, 0);
            messageLayout->setSpacing(0);

            QWidget* headerWidget = new QWidget(this);
            headerWidget->setAutoFillBackground(true);
            headerWidget->setStyleSheet(QStringLiteral("background: #%1; color: #%2; border-top: 1px solid transparent; border-top-left-radius: 4px; border-top-right-radius: 4px")
                                            .arg(QString::number(paidMessage["headerBackgroundColor"].toInteger(), 16),
                                                 QString::number(paidMessage["headerTextColor"].toInteger(), 16)));

            QHBoxLayout* headerLayout = new QHBoxLayout(headerWidget);
            headerLayout->setContentsMargins(5, 0, 0, 0);
            headerLayout->setSpacing(0);

            QLabel* authorIcon = new QLabel(this);
            authorIcon->setFixedSize(38, 32);
            headerLayout->addWidget(authorIcon);

            HttpReply* iconReply = Http::instance().get(paidMessage["authorPhoto"]["thumbnails"][0]["url"].toString());
            connect(iconReply, &HttpReply::finished, this, std::bind(&LiveChatWindow::setAuthorIcon, this, std::placeholders::_1, authorIcon));

            QVBoxLayout* innerHeaderLayout = new QVBoxLayout(headerWidget);
            innerHeaderLayout->setContentsMargins(0, 0, 0, 0);
            innerHeaderLayout->setSpacing(0);

            QLabel* authorLabel = new QLabel(paidMessage["authorName"]["simpleText"].toString(), this);
            authorLabel->setWordWrap(true);
            innerHeaderLayout->addWidget(authorLabel);

            QLabel* amountLabel = new QLabel(paidMessage["purchaseAmountText"]["simpleText"].toString(), this);
            amountLabel->setFont(QFont(qApp->font().toString(), -1, QFont::Bold));
            amountLabel->setWordWrap(true);
            innerHeaderLayout->addWidget(amountLabel);

            headerLayout->addLayout(innerHeaderLayout);
            messageLayout->addWidget(headerWidget);

            InnertubeObjects::InnertubeString message(paidMessage["message"]);
            if (!message.text.isEmpty())
            {
                QLabel* messageLabel = new QLabel(message.text, this);
                messageLabel->setAlignment(Qt::AlignCenter);
                messageLabel->setAutoFillBackground(true);
                messageLabel->setStyleSheet(
                    QStringLiteral("background: #%1; color: #%2; border-bottom: 1px solid transparent; border-bottom-left-radius: 4px; border-bottom-right-radius: 4px")
                        .arg(QString::number(paidMessage["bodyBackgroundColor"].toInteger(), 16),
                             QString::number(paidMessage["bodyTextColor"].toInteger(), 16)));
                messageLabel->setWordWrap(true);
                messageLayout->addWidget(messageLabel);
            }

            UIUtilities::addWidgetToList(ui->listWidget, messageWidget);
        }
        else if (item["liveChatSponsorshipsGiftRedemptionAnnouncementRenderer"].isObject())
        {
            QJsonValue announcement = item["liveChatSponsorshipsGiftRedemptionAnnouncementRenderer"];

            QWidget* announcementWidget = new QWidget(this);

            QHBoxLayout* announcementLayout = new QHBoxLayout(announcementWidget);
            announcementLayout->setContentsMargins(0, 0, 0, 0);
            announcementLayout->setSpacing(0);

            QLabel* authorLabel = new QLabel(announcement["authorName"]["simpleText"].toString(), this);
            authorLabel->setStyleSheet("font-weight: bold; color: #2ba640");
            announcementLayout->addWidget(authorLabel);

            InnertubeObjects::InnertubeString message(announcement["message"]);
            QLabel* messageLabel = new QLabel(" " + message.text, this);
            messageLabel->setFixedWidth(ui->listWidget->width() - 20);
            messageLabel->setFont(QFont(qApp->font().toString(), -1, -1, true));
            messageLabel->setWordWrap(true);
            announcementLayout->addWidget(messageLabel);

            UIUtilities::addWidgetToList(ui->listWidget, announcementWidget);
        }
        else if (item["liveChatViewerEngagementMessageRenderer"].isObject())
        {
            addSpecialMessage(item["liveChatViewerEngagementMessageRenderer"], "text", "message", false);
        }
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

    const QJsonArray continuations = liveChat.liveChatContinuation["continuations"].toArray();
    if (!continuations.isEmpty())
        currentContinuation = continuations[0]["invalidationContinuationData"]["continuation"].toString();

    timerRunning = false;
}

void LiveChatWindow::sendMessage()
{
    if (ui->messageBox->text().trimmed().isEmpty())
        return;

    QJsonValue sendLiveChatMessageEndpoint = actionPanel["liveChatMessageInputRenderer"]["sendButton"]["buttonRenderer"]
                                                        ["serviceEndpoint"]["sendLiveChatMessageEndpoint"];
    QString clientMessageId = sendLiveChatMessageEndpoint["clientIdPrefix"].toString() + QString::number(numSentMessages++);
    QString params = sendLiveChatMessageEndpoint["params"].toString();

    InnerTube::instance().get<InnertubeEndpoints::SendMessage>(emojicpp::emojize(ui->messageBox->text().trimmed()), clientMessageId, params);
    ui->messageBox->clear();
}

void LiveChatWindow::setAuthorIcon(const HttpReply& reply, QLabel* iconLabel)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    pixmap = pixmap.scaled(32, 32, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    iconLabel->setPixmap(UIUtilities::pixmapRounded(pixmap, 9999, 9999));
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
    messagesTimer->deleteLater();
    delete ui;
}
