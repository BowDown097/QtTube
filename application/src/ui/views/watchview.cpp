#include "watchview.hpp"
#include "watchview_ui.hpp"
#include "mainwindow.hpp"
#include "qttubeapplication.hpp"
#include "ui/forms/livechatwindow.hpp"
#include "ui/widgets/labels/channellabel.hpp"
#include "ui/widgets/labels/iconlabel.hpp"
#include "ui/widgets/modals/sharemodal.hpp"
#include "ui/widgets/subscribe/subscribewidget.hpp"
#include "ui/widgets/topbar/topbar.hpp"
#include "ui/widgets/watchnextfeed.hpp"
#include "utils/osutils.hpp"
#include "utils/uiutils.hpp"
#include "viewcontroller.hpp"
#include <QBoxLayout>
#include <QMessageBox>
#include <QProgressBar>
#include <QScrollArea>
#include <QScrollBar>
#include <QTimer>
#include <qttube-plugin/providers/providertypes.h>

WatchView::WatchView(const QString& videoId, PluginEntry* plugin, int progress,
                     PreloadData::WatchView* preload, QWidget* parent)
    : QWidget(parent), m_plugin(plugin), ui(new Ui::WatchView)
{
    assert(plugin->providers.watch != nullptr);

    MainWindow* mainWindow = UIUtils::getMainWindow();
    if (qtTubeApp->settings().autoHideTopBar)
    {
        mainWindow->topbar()->hide();
        mainWindow->topbar()->setAlwaysShow(false);
    }

    ui->setupUi(this, mainWindow, plugin);

    if (preload)
        processPreloadData(preload);

    QtTubePlugin::VideoReply* reply = plugin->providers.watch->getVideo(videoId);
    connect(reply, &QtTubePlugin::VideoReply::exception, this, &WatchView::loadFailed);
    connect(reply, &QtTubePlugin::VideoReply::finished, this, &WatchView::processData);

    ui->player->play(videoId, progress);
    connect(ui->description, &TubeLabel::linkActivated, this, &WatchView::descriptionLinkActivated);
}

WatchView::~WatchView()
{
    if (MainWindow* mainWindow = UIUtils::getMainWindow(false))
    {
        disconnect(mainWindow->topbar()->logo, &TubeLabel::clicked, this, nullptr);
        mainWindow->setWindowTitle(QTTUBE_APP_NAME);
    }

    OSUtils::suspendIdleSleep(false);
    delete ui;
}

void WatchView::descriptionLinkActivated(const QString& link)
{
    qtTubeApp->handleUrlOrID(link);
}

void WatchView::hotLoadVideo(
    const QString& videoId, int progress, PreloadData::WatchView* preload, bool continuePlayback)
{
    ui->scrollArea->horizontalScrollBar()->setValue(0);
    ui->scrollArea->verticalScrollBar()->setValue(0);

    if (continuePlayback)
    {
        ui->player->seek(progress);
        return;
    }

    ui->feed->reset();
    ui->setShowMore(false);

    if (m_metadataUpdateTimer)
        m_metadataUpdateTimer->deleteLater();

    UIUtils::clearLayout(ui->topLevelButtons);

    if (preload)
        processPreloadData(preload);

    QtTubePlugin::VideoReply* reply = m_plugin->providers.watch->getVideo(videoId);
    connect(reply, &QtTubePlugin::VideoReply::exception, this, &WatchView::loadFailed);
    connect(reply, &QtTubePlugin::VideoReply::finished, this, &WatchView::processData);

    ui->player->play(videoId, progress);
}

void WatchView::openLiveChat(const QtTubePlugin::InitialLiveChatData& data)
{
    LiveChatWindow* window = new LiveChatWindow(m_plugin);
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->show();
    window->initialize(data, ui->player);
}

void WatchView::processData(const QtTubePlugin::VideoData& data)
{
    m_channelId = data.channel.channelId;
    m_videoId = data.videoId;

    UIUtils::getMainWindow()->setWindowTitle(data.titleText % " - " % QTTUBE_APP_NAME);
    ui->channelLabel->setInfo(m_channelId, data.channel.channelName, data.channel.channelBadges);
    ui->date->setText(data.dateText);
    ui->subscribeWidget->setData(data.channel.channelId, data.channel.subscribeButton);
    ui->titleLabel->setText(data.titleText);
    ui->viewCount->setText(data.viewCountText);

    ui->channelIcon->setImage(data.channel.channelAvatarUrl);
    connect(ui->channelIcon, &TubeLabel::clicked, this, [this] {
        ViewController::loadChannel(m_channelId, m_plugin);
    });

    ui->description->setText(data.descriptionText);
    ui->description->setVisible(!data.descriptionText.isEmpty());
    ui->showMoreLabel->setVisible(
        ui->description->heightForWidth(ui->description->width()) >
        ui->description->maximumHeight());

    IconLabel* shareLabel = new IconLabel("share", "Share", QMargins(5, 0, 0, 0));
    ui->topLevelButtons->addWidget(shareLabel);
    connect(shareLabel, &IconLabel::clicked, this, &WatchView::showShareModal);

    if (data.initialLiveChatData.has_value())
    {
        IconLabel* liveChatLabel = new IconLabel("live-chat", "Chat", QMargins(15, 0, 0, 0));
        ui->topLevelButtons->addWidget(liveChatLabel);
        connect(liveChatLabel, &IconLabel::clicked, this,
            std::bind(&WatchView::openLiveChat, this, data.initialLiveChatData.value()));
    }

    ui->topLevelButtons->addStretch();

    if (data.ratingsAvailable)
    {
        ui->likeLabel = new IconLabel("like", QMargins(0, 0, 15, 0));
        ui->likeLabel->setText(data.likeCountText);
        ui->topLevelButtons->addWidget(ui->likeLabel);
        connect(ui->likeLabel, &IconLabel::clicked, this,
            std::bind(&WatchView::rate, this, true, data.likeData.like, data.likeData.removeLike));
        if (data.likeStatus == QtTubePlugin::VideoData::LikeStatus::Liked)
        {
            ui->likeLabel->setIcon("like-toggled");
            ui->likeLabel->setStyleSheet("color: #167ac6");
        }

        ui->dislikeLabel = new IconLabel("dislike");
        ui->dislikeLabel->setText(data.dislikeCountText);
        ui->topLevelButtons->addWidget(ui->dislikeLabel);
        connect(ui->dislikeLabel, &IconLabel::clicked, this,
            std::bind(&WatchView::rate, this, false, data.likeData.dislike, data.likeData.removeDislike));
        if (data.likeStatus == QtTubePlugin::VideoData::LikeStatus::Disliked)
        {
            ui->dislikeLabel->setIcon("dislike-toggled");
            ui->dislikeLabel->setStyleSheet("color: #167ac6");
        }

        if (data.likeDislikeRatio > 0)
        {
            if (!ui->likeBar)
                ui->constructLikeBar();
            ui->likeBar->setValue(data.likeDislikeRatio * 100);
        }
    }

    if (data.isLiveContent)
    {
        m_metadataUpdateTimer = new QTimer(this);
        m_metadataUpdateTimer->setInterval(60000);
        connect(m_metadataUpdateTimer, &QTimer::timeout, this, std::bind(&WatchView::updateMetadata, this, data.videoId));
        m_metadataUpdateTimer->start();
    }

    ui->feed->setData(data.videoId, data.recommendedVideos, data.continuations);
}

void WatchView::processPreloadData(PreloadData::WatchView* preload)
{
    ui->channelIcon->setImage(preload->channelAvatarUrl);
    ui->titleLabel->setText(preload->title);
    if (!preload->channelId.isEmpty() && !preload->channelName.isEmpty())
        ui->channelLabel->setInfo(preload->channelId, preload->channelName, preload->channelBadges);
    if (!preload->title.isEmpty())
        ui->titleLabel->setText(preload->title);
}

void WatchView::rate(bool like, const std::any& addData, const std::any& removeData)
{
    IconLabel* senderLabel = qobject_cast<IconLabel*>(sender());

    bool textIsNumber;
    qint64 count = QLocale::system().toLongLong(senderLabel->text(), &textIsNumber);

    if (senderLabel->styleSheet().isEmpty()) // if untoggled
    {
        senderLabel->setIcon(like ? "like-toggled" : "dislike-toggled");
        senderLabel->setStyleSheet("color: #167ac6");
        if (textIsNumber)
            senderLabel->setText(QLocale::system().toString(count + 1));

        if (!m_plugin->providers.watch->rate(m_videoId, like, false, addData))
            QMessageBox::warning(nullptr, "Feature Not Available", "This feature is not supported by the active plugin.");
    }
    else
    {
        senderLabel->setIcon(like ? "like" : "dislike");
        senderLabel->setStyleSheet(QString());
        if (textIsNumber)
            senderLabel->setText(QLocale::system().toString(count - 1));

        if (!m_plugin->providers.watch->rate(m_videoId, like, true, removeData))
            QMessageBox::warning(nullptr, "Feature Not Available", "This feature is not supported by the active plugin.");
    }
}

void WatchView::resizeEvent(QResizeEvent* event)
{
    if (!ui->primaryInfoWrapper || !event->oldSize().isValid())
        return;

    ui->player->calcAndSetSize(event->size());
    ui->scrollArea->setMaximumHeight(event->size().height());

    int width = ui->player->size().width();
    ui->description->setFixedWidth(width);
    ui->feed->setMaximumWidth(ui->player->scaleMode() == WatchViewPlayer::ScaleMode::Scaled
        ? event->size().width() - width - ui->primaryLayout->spacing() : QWIDGETSIZE_MAX);
    ui->menuWrapper->setFixedWidth(width);
    ui->primaryInfoWrapper->setFixedWidth(width);
    ui->scrollArea->setMaximumWidth(width);
    ui->showMoreLabel->setFixedWidth(width);
    ui->titleLabel->setFixedWidth(width);
}

void WatchView::showShareModal()
{
    new ShareModal(m_plugin->metadata.videoUrlTemplate, m_videoId, UIUtils::getMainWindow());
}

void WatchView::updateMetadata(const QString& videoId)
{
    QtTubePlugin::VideoReply* reply = m_plugin->providers.watch->getVideo(videoId);
    connect(reply, &QtTubePlugin::VideoReply::exception, this, [this](const QtTubePlugin::Exception& ex) {
        qDebug() << ex.message() << "Stream/premiere could have ended - killing update timer.";
        m_metadataUpdateTimer->deleteLater();
    });
    connect(reply, &QtTubePlugin::VideoReply::finished, this, [this](const QtTubePlugin::VideoData& data) {
        ui->date->setText(data.dateText);
        ui->description->setText(data.descriptionText);
        ui->titleLabel->setText(data.titleText);
        ui->viewCount->setText(data.viewCountText);

        if (data.ratingsAvailable)
        {
            ui->dislikeLabel->setText(data.dislikeCountText);
            ui->likeLabel->setText(data.likeCountText);
            if (data.likeDislikeRatio > 0)
                ui->likeBar->setValue(data.likeDislikeRatio * 100);
        }
    });
}
