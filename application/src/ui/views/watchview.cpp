#include "watchview.h"
#include "watchview_ui.h"
#include "mainwindow.h"
#include "qttubeapplication.h"
#include "ui/forms/livechatwindow.h"
#include "ui/views/viewcontroller.h"
#include "ui/widgets/labels/channellabel.h"
#include "ui/widgets/labels/iconlabel.h"
#include "ui/widgets/modals/sharemodal.h"
#include "ui/widgets/subscribe/subscribewidget.h"
#include "ui/widgets/watchnextfeed.h"
#include "utils/osutils.h"
#include "utils/uiutils.h"
#include <QBoxLayout>
#include <QDesktopServices>
#include <QProgressBar>
#include <QScrollArea>
#include <QScrollBar>
#include <QTimer>
#include <QUrlQuery>

WatchView::WatchView(const QString& videoId, int progress, PreloadData::WatchView* preload, QWidget* parent)
    : QWidget(parent), ui(new Ui::WatchView)
{
    if (qtTubeApp->settings().autoHideTopBar)
    {
        MainWindow::topbar()->hide();
        MainWindow::topbar()->setAlwaysShow(false);
    }

    ui->setupUi(this);

    if (preload)
        processPreloadData(preload);

    if (const PluginData* plugin = qtTubeApp->plugins().activePlugin())
    {
        QtTube::VideoReply* reply = plugin->interface->getVideo(videoId, {});
        connect(reply, &QtTube::VideoReply::exception, this, &WatchView::loadFailed);
        connect(reply, &QtTube::VideoReply::finished, this, &WatchView::processData);
    }

    ui->player->play(videoId, progress);
    connect(ui->description, &TubeLabel::linkActivated, this, &WatchView::descriptionLinkActivated);
}

WatchView::~WatchView()
{
    disconnect(MainWindow::topbar()->logo, &TubeLabel::clicked, this, nullptr);
    OSUtils::suspendIdleSleep(false);

    if (QMainWindow* mainWindow = UIUtils::getMainWindow())
        mainWindow->setWindowTitle(QTTUBE_APP_NAME);

    delete ui;
}

void WatchView::descriptionLinkActivated(const QString& url)
{
    QUrl qUrl(url);
    if (url.startsWith("http"))
    {
        QDesktopServices::openUrl(qUrl);
    }
    else if (url.startsWith("/channel"))
    {
        QString funnyPath = qUrl.path().replace("/channel/", "");
        ViewController::loadChannel(funnyPath.left(funnyPath.indexOf('/')));
    }
    else if (url.startsWith("/watch"))
    {
        ui->scrollArea->verticalScrollBar()->setValue(0);

        QUrlQuery query(qUrl);
        int progress = query.queryItemValue("t").replace("s", "").toInt();

        if (query.queryItemValue("continuePlayback") == "1")
        {
            ui->player->seek(progress);
        }
        else
        {
            hotLoadVideo(query.queryItemValue("v"), progress);
            ui->toggleShowMore();
        }
    }
    else
    {
        qDebug() << "Ran into unsupported description link:" << url;
    }
}

void WatchView::hotLoadVideo(const QString& videoId, int progress, PreloadData::WatchView* preload)
{
    ui->feed->reset();
    ui->scrollArea->horizontalScrollBar()->setValue(0);
    ui->scrollArea->verticalScrollBar()->setValue(0);

    if (metadataUpdateTimer)
        metadataUpdateTimer->deleteLater();

    UIUtils::clearLayout(ui->topLevelButtons);
    disconnect(ui->channelLabel->text, &TubeLabel::clicked, nullptr, nullptr);

    if (preload)
        processPreloadData(preload);

    if (const PluginData* plugin = qtTubeApp->plugins().activePlugin())
    {
        QtTube::VideoReply* reply = plugin->interface->getVideo(videoId, {});
        connect(reply, &QtTube::VideoReply::exception, this, &WatchView::loadFailed);
        connect(reply, &QtTube::VideoReply::finished, this, &WatchView::processData);
    }

    ui->player->play(videoId, progress);
}

void WatchView::openLiveChat(const QtTube::InitialLiveChatData& data)
{
    LiveChatWindow* window = new LiveChatWindow;
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->show();
    window->initialize(data, ui->player);
}

void WatchView::processData(const QtTube::VideoData& data)
{
    videoId = data.videoId;
    videoUrlPrefix = data.videoUrlPrefix;

    if (QMainWindow* mainWindow = UIUtils::getMainWindow())
        mainWindow->setWindowTitle(data.titleText + " - " + QTTUBE_APP_NAME);

    ui->channelIcon->setImage(data.channel.channelAvatarUrl);
    ui->channelLabel->setInfo(data.channel.channelId, data.channel.channelName, data.channel.channelBadges);
    ui->date->setText(data.dateText);
    ui->subscribeWidget->setData(data.channel.subscribeButton);
    ui->titleLabel->setText(data.titleText);
    ui->viewCount->setText(data.viewCountText);

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
        if (data.likeStatus == QtTube::VideoData::LikeStatus::Liked)
        {
            ui->likeLabel->setIcon("like-toggled");
            ui->likeLabel->setStyleSheet("color: #167ac6");
        }

        ui->dislikeLabel = new IconLabel("dislike");
        ui->dislikeLabel->setText(data.dislikeCountText);
        ui->topLevelButtons->addWidget(ui->dislikeLabel);
        connect(ui->dislikeLabel, &IconLabel::clicked, this,
            std::bind(&WatchView::rate, this, false, data.likeData.dislike, data.likeData.removeDislike));
        if (data.likeStatus == QtTube::VideoData::LikeStatus::Disliked)
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
        metadataUpdateTimer = new QTimer(this);
        metadataUpdateTimer->setInterval(60000);
        connect(metadataUpdateTimer, &QTimer::timeout, this, std::bind(&WatchView::updateMetadata, this, data.videoId));
        metadataUpdateTimer->start();
    }

    ui->feed->setData(data.recommendedVideos, data.continuations);
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

        if (const PluginData* plugin = qtTubeApp->plugins().activePlugin())
            plugin->interface->rate(videoId, like, false, addData);
    }
    else
    {
        senderLabel->setIcon(like ? "like" : "dislike");
        senderLabel->setStyleSheet(QString());
        if (textIsNumber)
            senderLabel->setText(QLocale::system().toString(count - 1));

        if (const PluginData* plugin = qtTubeApp->plugins().activePlugin())
            plugin->interface->rate(videoId, like, true, removeData);
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
    new ShareModal(videoUrlPrefix, videoId, UIUtils::getMainWindow());
}

void WatchView::updateMetadata(const QString& videoId)
{
    if (const PluginData* plugin = qtTubeApp->plugins().activePlugin())
    {
        QtTube::VideoReply* reply = plugin->interface->getVideo(videoId, {});
        connect(reply, &QtTube::VideoReply::exception, this, [this](const QtTube::PluginException& ex) {
            qDebug() << ex.message() << "Stream/premiere could have ended - killing update timer.";
            metadataUpdateTimer->deleteLater();
        });
        connect(reply, &QtTube::VideoReply::finished, this, [this](const QtTube::VideoData& data) {
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
}
