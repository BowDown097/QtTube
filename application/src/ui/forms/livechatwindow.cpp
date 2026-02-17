#include "livechatwindow.h"
#include "ui_livechatwindow.h"
#include "plugins/pluginmanager.h"
#include "stores/emojistore.h"
#include "ui/widgets/emoji/emojimenu.h"
#include "ui/widgets/labels/tubelabel.h"
#include "ui/widgets/livechat/giftredemptionmessage.h"
#include "ui/widgets/livechat/paidmessage.h"
#include "ui/widgets/livechat/specialmessage.h"
#include "ui/widgets/livechat/textmessage.h"
#include "utils/uiutils.h"
#include <QMessageBox>
#include <QTimer>

LiveChatWindow::LiveChatWindow(PluginData* plugin, QWidget* parent)
    : QWidget(parent), m_messagesTimer(new QTimer(this)), m_plugin(plugin), ui(new Ui::LiveChatWindow)
{
    ui->setupUi(this);
    ui->chatModeSwitcher->hide();

    connect(ui->messageBox, &QLineEdit::returnPressed, this, &LiveChatWindow::sendMessage);
    connect(ui->sendButton, &QPushButton::pressed, this, &LiveChatWindow::sendMessage);
}

LiveChatWindow::~LiveChatWindow()
{
    // avoid use-after-free when closing
    if (m_populating)
        waitForPopulation();

    delete ui;
}

void LiveChatWindow::addChatItemToList(const QtTubePlugin::LiveChatItem& item)
{
    if (const auto* giftRedemption = std::get_if<QtTubePlugin::GiftRedemptionMessage>(&item))
        UIUtils::addWidgetToList(ui->listWidget, new GiftRedemptionMessage(*giftRedemption, this));
    else if (const auto* paidMessage = std::get_if<QtTubePlugin::PaidMessage>(&item))
        UIUtils::addWidgetToList(ui->listWidget, new PaidMessage(*paidMessage, this));
    else if (const auto* specialMessage = std::get_if<QtTubePlugin::SpecialMessage>(&item))
        UIUtils::addWidgetToList(ui->listWidget, new SpecialMessage(*specialMessage, this));
    else if (const auto* textMessage = std::get_if<QtTubePlugin::TextMessage>(&item))
        UIUtils::addWidgetToList(ui->listWidget, new TextMessage(*textMessage, this));
}

void LiveChatWindow::addNewChatReplayItems(qint64 progress, qint64 previousProgress, bool seeked)
{
    for (auto it = m_replayItems.begin(); it != m_replayItems.end();)
    {
        qint64 videoOffsetSec = it->videoOffsetMs / 1000;
        if ((seeked || videoOffsetSec >= previousProgress) && videoOffsetSec <= progress)
        {
            addChatItemToList(it->item);
            it = m_replayItems.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void LiveChatWindow::chatModeChanged(const QString& name)
{
    if (auto it = m_viewOptions.find(name); it != m_viewOptions.end())
    {
        if (!it->second.has_value())
        {
            QMessageBox::critical(this, "Failed to Switch Chat Mode", "Required data is missing or unavailable.");
            return;
        }

        if (m_populating)
            waitForPopulation();

        m_nextData = it->second;
        ui->listWidget->clear();
    }
}

void LiveChatWindow::chatReplayTick(qint64 progress, qint64 previousProgress)
{
    if (!m_populating)
    {
        m_populating = true;
        if (previousProgress > 0 && std::abs(progress - previousProgress) > 5)
        {
            ui->listWidget->clear();
            if (QtTubePlugin::LiveChatReplayReply* reply = m_plugin->interface->getLiveChatReplay(m_seekData, progress * 1000))
            {
                connect(reply, &QtTubePlugin::LiveChatReplayReply::finished, this,
                    std::bind_front(&LiveChatWindow::processChatReplayData, this, progress, previousProgress, true));
            }
            else
            {
                QMessageBox::critical(nullptr, "Feature Not Available", "This feature is not supported by the active plugin.");
                deleteLater();
            }
        }
        else if (progress < m_firstChatItemOffset || progress > m_lastChatItemOffset)
        {
            if (QtTubePlugin::LiveChatReplayReply* reply = m_plugin->interface->getLiveChatReplay(m_nextData, progress * 1000))
            {
                connect(reply, &QtTubePlugin::LiveChatReplayReply::finished, this,
                    std::bind_front(&LiveChatWindow::processChatReplayData, this, progress, previousProgress, false));
            }
            else
            {
                QMessageBox::critical(nullptr, "Feature Not Available", "This feature is not supported by the active plugin.");
                deleteLater();
            }
        }
        else
        {
            updateChatReplay(progress, previousProgress);
        }
    }
}

void LiveChatWindow::chatTick()
{
    if (!m_populating)
    {
        m_populating = true;
        if (QtTubePlugin::LiveChatReply* reply = m_plugin->interface->getLiveChat(m_nextData))
        {
            connect(reply, &QtTubePlugin::LiveChatReply::finished, this, &LiveChatWindow::processChatData);
        }
        else
        {
            QMessageBox::critical(nullptr, "Feature Not Available", "This feature is not supported by the active plugin.");
            deleteLater();
        }
    }
}

void LiveChatWindow::createEmojiMenuWidgets()
{
    EmojiMenu* emojiMenu = new EmojiMenu(this, Qt::Window);
    connect(emojiMenu, &EmojiMenu::emojiClicked, this, &LiveChatWindow::insertEmoji);

    TubeLabel* emojiMenuLabel = new TubeLabel(this);
    emojiMenuLabel->setClickable(true);
    emojiMenuLabel->setFixedSize(ui->messageBox->height() - 8, ui->messageBox->height() - 8);
    emojiMenuLabel->setScaledContents(true);
    emojiMenuLabel->setPixmap(UIUtils::pixmapThemed("emoji"));
    ui->horizontalLayout->insertWidget(0, emojiMenuLabel);
    connect(emojiMenuLabel, &TubeLabel::clicked, emojiMenu, &EmojiMenu::show);
}

void LiveChatWindow::initialize(const QtTubePlugin::InitialLiveChatData& data, WatchViewPlayer* player)
{
    m_nextData = data.data;
    if (data.isReplay)
    {
        ui->chatModeSwitcher->hide();
        ui->messageBox->hide();
        ui->sendButton->hide();
        if (player)
            connect(player, &WatchViewPlayer::progressChanged, this, &LiveChatWindow::chatReplayTick);
    }
    else
    {
        createEmojiMenuWidgets();
        if (!data.platformEmojis.isEmpty())
            EmojiStore::instance()->add("Platform", data.platformEmojis, false);
        m_messagesTimer->start(data.updateIntervalMs);
        connect(m_messagesTimer, &QTimer::timeout, this, &LiveChatWindow::chatTick);
    }
}

void LiveChatWindow::insertEmoji(const QtTubePlugin::Emoji& emoji)
{
    const QString msg = ui->messageBox->text();
    const QString& shortcode = emoji.shortcodes.front();
    bool needsSpace = !msg.isEmpty() && !msg[ui->messageBox->cursorPosition() - 1].isSpace();
    ui->messageBox->insert(needsSpace ? ' ' + shortcode : shortcode);
}

void LiveChatWindow::processChatData(const QtTubePlugin::LiveChat& data)
{
    bool isRestricted = !data.restrictedMessage.isEmpty();
    ui->messageBox->setEnabled(!isRestricted);
    ui->messageBox->setPlaceholderText(isRestricted ? data.restrictedMessage : "Say something...");
    ui->sendButton->setEnabled(!isRestricted);

    if (ui->chatModeSwitcher->isHidden() && !data.viewOptions.isEmpty())
    {
        for (const QtTubePlugin::LiveChatViewOption& option : data.viewOptions)
        {
            m_viewOptions.emplace(option.name, option.data);
            ui->chatModeSwitcher->addItem(option.name);
        }

        ui->chatModeSwitcher->show();
        connect(ui->chatModeSwitcher, &QComboBox::currentTextChanged, this, &LiveChatWindow::chatModeChanged);
    }

    for (const QtTubePlugin::LiveChatItem& item : data.items)
        addChatItemToList(item);

    if (data.nextData.has_value())
        m_nextData = data.nextData;
    else
        m_messagesTimer->stop();

    processingEnd();
}

void LiveChatWindow::processChatReplayData(
    qint64 progress, qint64 previousProgress, bool seeked, const QtTubePlugin::LiveChatReplay& data)
{
    if (ui->chatModeSwitcher->isHidden() && !data.viewOptions.isEmpty())
    {
        for (const QtTubePlugin::LiveChatViewOption& option : data.viewOptions)
        {
            m_viewOptions.emplace(option.name, option.data);
            ui->chatModeSwitcher->addItem(option.name);
        }

        ui->chatModeSwitcher->show();
        connect(ui->chatModeSwitcher, &QComboBox::currentTextChanged, this, &LiveChatWindow::chatModeChanged);
    }

    m_replayItems = data.items;
    addNewChatReplayItems(progress, previousProgress, seeked);

    if (data.nextData.has_value())
        m_nextData = data.nextData;

    m_firstChatItemOffset = m_replayItems.first().videoOffsetMs / 1000;
    m_lastChatItemOffset = m_replayItems.last().videoOffsetMs / 1000;
    m_seekData = data.seekData;

    processingEnd();
}

void LiveChatWindow::processingEnd()
{
    if (ui->listWidget->count() > 250)
        for (int i = 0; i < ui->listWidget->count() - 250; i++)
            delete ui->listWidget->takeItem(i);

    ui->listWidget->scrollToBottom();
    m_populating = false;
    emit getLiveChatFinished();
}

void LiveChatWindow::sendMessage()
{
    if (QString trimmedText = ui->messageBox->text().trimmed(); !trimmedText.isEmpty())
    {
        if (m_plugin->interface->sendLiveChatMessage(EmojiStore::instance()->emojize(trimmedText)))
            ui->messageBox->clear();
        else
            QMessageBox::warning(nullptr, "Feature Not Available", "This feature is not supported by the active plugin.");
    }
}

void LiveChatWindow::updateChatReplay(qint64 progress, qint64 previousProgress)
{
    addNewChatReplayItems(progress, previousProgress, false);
    processingEnd();
}

void LiveChatWindow::waitForPopulation()
{
    QEventLoop loop;
    connect(this, &LiveChatWindow::getLiveChatFinished, &loop, &QEventLoop::quit);
    loop.exec();
}
