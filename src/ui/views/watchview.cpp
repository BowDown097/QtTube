#include "watchview.h"
#include "ui/views/viewcontroller.h"
#include "watchview_ui.h"
#include "http.h"
#include "innertube.h"
#include "mainwindow.h"
#include "preloaddata.h"
#include "qttubeapplication.h"
#include "ui/forms/livechat/livechatwindow.h"
#include "ui/widgets/labels/channellabel.h"
#include "ui/widgets/labels/iconlabel.h"
#include "ui/widgets/subscribe/subscribewidget.h"
#include "ui/widgets/watchnextfeed.h"
#include "utils/osutils.h"
#include "utils/stringutils.h"
#include "utils/uiutils.h"
#include <QBoxLayout>
#include <QDesktopServices>
#include <QJsonDocument>
#include <QProgressBar>
#include <QScrollArea>
#include <QScrollBar>
#include <QTimer>
#include <QUrlQuery>

WatchView::~WatchView()
{
    disconnect(MainWindow::topbar()->logo, &TubeLabel::clicked, this, nullptr);
    OSUtils::suspendIdleSleep(false);

    if (QMainWindow* mainWindow = qobject_cast<QMainWindow*>(qApp->activeWindow()))
        mainWindow->setWindowTitle(QTTUBE_APP_NAME);
}

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

    auto next = InnerTube::instance()->get<InnertubeEndpoints::Next>(videoId);
    connect(next, &InnertubeReply<InnertubeEndpoints::Next>::finished, this, &WatchView::processNext);
    connect(next, &InnertubeReply<InnertubeEndpoints::Next>::exception, this, &WatchView::loadFailed);

    auto player = InnerTube::instance()->get<InnertubeEndpoints::Player>(videoId);
    connect(player, &InnertubeReply<InnertubeEndpoints::Player>::finished, this, &WatchView::processPlayer);
    connect(player, &InnertubeReply<InnertubeEndpoints::Player>::exception, this, &WatchView::loadFailed);

    ui->player->play(videoId, progress);
    connect(ui->description, &TubeLabel::linkActivated, this, &WatchView::descriptionLinkActivated);
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

    auto next = InnerTube::instance()->get<InnertubeEndpoints::Next>(videoId);
    connect(next, &InnertubeReply<InnertubeEndpoints::Next>::finished, this, &WatchView::processNext);
    connect(next, &InnertubeReply<InnertubeEndpoints::Next>::exception, this, &WatchView::loadFailed);

    auto player = InnerTube::instance()->get<InnertubeEndpoints::Player>(videoId);
    connect(player, &InnertubeReply<InnertubeEndpoints::Player>::finished, this, &WatchView::processPlayer);
    connect(player, &InnertubeReply<InnertubeEndpoints::Player>::exception, this, &WatchView::loadFailed);

    ui->player->play(videoId, progress);
}

void WatchView::likeOrDislike(bool like, const InnertubeObjects::ToggleButtonViewModel& toggleButton)
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

        QJsonValue command = toggleButton.defaultButtonViewModel.onTap["serialCommand"]["commands"][1]["innertubeCommand"];
        InnerTube::instance()->like(command["likeEndpoint"], like);
    }
    else
    {
        senderLabel->setIcon(like ? "like" : "dislike");
        senderLabel->setStyleSheet(QString());
        if (textIsNumber)
            senderLabel->setText(QLocale::system().toString(count - 1));

        QJsonValue command = toggleButton.toggledButtonViewModel.onTap["serialCommand"]["commands"][1]["innertubeCommand"];
        InnerTube::instance()->like(command["likeEndpoint"], like);
    }
}

void WatchView::processNext(const InnertubeEndpoints::Next& endpoint)
{
    const InnertubeEndpoints::NextResponse& nextResp = endpoint.response;
    const InnertubeObjects::VideoPrimaryInfo& primaryInfo = nextResp.results.results.primaryInfo;
    const InnertubeObjects::VideoSecondaryInfo& secondaryInfo = nextResp.results.results.secondaryInfo;
    channelId = nextResp.results.results.secondaryInfo.owner.navigationEndpoint["browseEndpoint"]["browseId"].toString();

    ui->channelLabel->setInfo(channelId, secondaryInfo.owner.title.text, secondaryInfo.owner.badges);

    ui->subscribeWidget->setSubscribeButton(secondaryInfo.subscribeButton);
    ui->subscribeWidget->setSubscriberCount(secondaryInfo.owner.subscriberCountText.text, secondaryInfo.subscribeButton.channelId);
    ui->subscribeWidget->subscribersCountLabel->show();
    ui->viewCount->setText(qtTubeApp->settings().condensedCounts && !primaryInfo.viewCount.isLive
                               ? primaryInfo.viewCount.extraShortViewCount.text + " views"
                               : primaryInfo.viewCount.viewCount.text);

    for (const InnertubeObjects::MenuFlexibleItem& fi : primaryInfo.videoActions.flexibleItems)
    {
        // these will never be implemented
        if (fi.topLevelButton.iconName == "CONTENT_CUT" || fi.topLevelButton.iconName == "MONEY_HEART")
            continue;

        // change "Save" button to "Add to" to replicate Hitchhiker style
        QString title = fi.topLevelButton.title;
        if (title == "Save")
            title = "Add to";

        ui->topLevelButtons->addWidget(new IconLabel(
            fi.topLevelButton.iconName.toLower(),
            title,
            ui->topLevelButtons->count() > 0 ? QMargins(15, 0, 0, 0) : QMargins(5, 0, 0, 0)
        ));
    }

    IconLabel* shareLabel = new IconLabel("share", "Share", ui->topLevelButtons->count() > 0 ? QMargins(15, 0, 0, 0) : QMargins(5, 0, 0, 0));
    ui->topLevelButtons->addWidget(shareLabel);

    if (std::optional<InnertubeObjects::LiveChat> liveChat = nextResp.results.liveChat; liveChat.has_value())
    {
        IconLabel* liveChatLabel = new IconLabel("live-chat", "Chat", QMargins(15, 0, 0, 0));
        ui->topLevelButtons->addWidget(liveChatLabel);

        connect(liveChatLabel, &IconLabel::clicked, this, [this, liveChat]
        {
            LiveChatWindow* liveChatWindow = new LiveChatWindow;
            liveChatWindow->setAttribute(Qt::WA_DeleteOnClose);
            liveChatWindow->show();
            liveChatWindow->initialize(liveChat.value(), ui->player);
        });
    }

    ui->topLevelButtons->addStretch();

    const InnertubeObjects::LikeDislikeViewModel& likeDislikeViewModel = primaryInfo.videoActions.segmentedLikeDislikeButtonViewModel;

    ui->likeLabel = new IconLabel("like", QMargins(0, 0, 15, 0));
    ui->topLevelButtons->addWidget(ui->likeLabel);
    connect(ui->likeLabel, &IconLabel::clicked, this,
            std::bind(&WatchView::likeOrDislike, this, true, likeDislikeViewModel.likeButtonViewModel.toggleButtonViewModel));
    if (likeDislikeViewModel.likeButtonViewModel.likeStatus == "LIKE")
    {
        ui->likeLabel->setIcon("like-toggled");
        ui->likeLabel->setStyleSheet("color: #167ac6");
    }

    ui->dislikeLabel = new IconLabel("dislike");
    ui->topLevelButtons->addWidget(ui->dislikeLabel);
    connect(ui->dislikeLabel, &IconLabel::clicked, this,
            std::bind(&WatchView::likeOrDislike, this, false, likeDislikeViewModel.dislikeButtonViewModel.toggleButtonViewModel));
    if (likeDislikeViewModel.likeButtonViewModel.likeStatus == "DISLIKE")
    {
        ui->dislikeLabel->setIcon("dislike-toggled");
        ui->dislikeLabel->setStyleSheet("color: #167ac6");
    }

    if (const InnertubeObjects::GenericThumbnail* recThumbnail = secondaryInfo.owner.thumbnail.recommendedQuality(QSize(48, 48)))
    {
        HttpReply* reply = Http::instance().get(recThumbnail->url);
        connect(reply, &HttpReply::finished, this, &WatchView::setChannelIcon);
    }

    const InnertubeObjects::ButtonViewModel& likeViewModel = likeDislikeViewModel.likeButtonViewModel.toggleButtonViewModel.defaultButtonViewModel;
    ui->likeLabel->setProperty("fullCount", StringUtils::extractDigits(likeViewModel.accessibilityText));
    ui->likeLabel->setText(qtTubeApp->settings().condensedCounts
        ? likeViewModel.title : ui->likeLabel->property("fullCount").toString());

    if (qtTubeApp->settings().returnDislikes)
    {
        HttpReply* reply = Http::instance().get("https://returnyoutubedislikeapi.com/votes?videoId=" + nextResp.videoId);
        connect(reply, &HttpReply::finished, this, &WatchView::setDislikes);
    }
    else
    {
        ui->dislikeLabel->setText("Dislike");
    }

    // Add "Published on" before date of normal videos to replicate Hitchhiker style and add super title after the date
    QString dateText = primaryInfo.dateText.text;
    if (!dateText.startsWith("Premier") && !dateText.startsWith("St") && !dateText.startsWith("Sched"))
        dateText.prepend("Published on ");
    if (nextResp.results.results.primaryInfo.superTitleLink.has_value())
        dateText += " | " + StringUtils::innertubeStringToRichText(nextResp.results.results.primaryInfo.superTitleLink.value(), true);

    ui->date->setText(dateText);
    ui->description->setText(StringUtils::innertubeStringToRichText(unattributeDescription(nextResp.results.results.secondaryInfo.attributedDescription), false));
    ui->description->setVisible(!ui->description->text().isEmpty());
    ui->showMoreLabel->setVisible(ui->description->heightForWidth(ui->description->width()) > ui->description->maximumHeight());
    ui->feed->setData(endpoint);
}

void WatchView::processPlayer(const InnertubeEndpoints::Player& endpoint)
{
    const InnertubeEndpoints::PlayerResponse& playerResp = endpoint.response;
    ui->player->startTracking(playerResp);
    ui->titleLabel->setText(playerResp.videoDetails.title);

    if (QMainWindow* mainWindow = qobject_cast<QMainWindow*>(qApp->activeWindow()))
        mainWindow->setWindowTitle(playerResp.videoDetails.title + " - " + QTTUBE_APP_NAME);

    if (playerResp.videoDetails.isLive || playerResp.videoDetails.isUpcoming)
    {
        metadataUpdateTimer = new QTimer(this);
        metadataUpdateTimer->setInterval(60000);
        connect(metadataUpdateTimer, &QTimer::timeout, this, std::bind(&WatchView::updateMetadata, this, playerResp.videoDetails.videoId));
        metadataUpdateTimer->start();
    }
}

void WatchView::processPreloadData(PreloadData::WatchView* preload)
{
    if (preload->channelAvatar.has_value())
    {
        if (const InnertubeObjects::GenericThumbnail* recAvatar = preload->channelAvatar->recommendedQuality(QSize(48, 48)))
        {
            HttpReply* reply = Http::instance().get(recAvatar->url);
            connect(reply, &HttpReply::finished, this, &WatchView::setChannelIcon);
        }
    }

    if (preload->channelId.has_value() && preload->channelName.has_value())
        ui->channelLabel->setInfo(preload->channelId.value(), preload->channelName.value(), preload->channelBadges);
    if (preload->title.has_value())
        ui->titleLabel->setText(preload->title.value());
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

void WatchView::setChannelIcon(const HttpReply& reply)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    ui->channelIcon->setPixmap(pixmap);
}

void WatchView::setDislikes(const HttpReply& reply)
{
    if (!reply.isSuccessful())
    {
        if (ui->dislikeLabel->text().isEmpty())
            ui->dislikeLabel->setText("Dislike");
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply.body());
    qint64 dislikes = doc["dislikes"].toVariant().toLongLong();
    qint64 likes = QLocale::system().toLongLong(ui->likeLabel->property("fullCount").toString());

    if (likes != 0)
    {
        ui->likeBar->setMaximum(likes + dislikes);
        ui->likeBar->setValue(likes);
    }

    ui->likeBar->show();

#ifdef QTTUBE_HAS_ICU
    if (qtTubeApp->settings().condensedCounts)
    {
        ui->dislikeLabel->setText(StringUtils::condensedNumericString(dislikes));
        return;
    }
#endif

    ui->dislikeLabel->setText(QLocale::system().toString(dislikes));
}

// most logic courtesy of https://github.com/Rehike/Rehike
InnertubeObjects::InnertubeString WatchView::unattributeDescription(const InnertubeObjects::DynamicText& attributedDescription)
{
    const QString& content = attributedDescription.content;
    if (!attributedDescription.commandRuns.isArray())
        return InnertubeObjects::InnertubeString(content);

    const QJsonArray commandRuns = attributedDescription.commandRuns.toArray();
    InnertubeObjects::InnertubeString out;
    int start = 0;

    for (const QJsonValue& commandRun : commandRuns)
    {
        int length = commandRun["length"].toInt();
        int startIndex = commandRun["startIndex"].toInt();

        if (QString beforeText = content.mid(start, startIndex - start); !beforeText.isEmpty())
            out.runs.append(InnertubeObjects::InnertubeRun(beforeText));

        QString linkText = content.mid(startIndex, length);
        out.runs.append(InnertubeObjects::InnertubeRun(linkText, commandRun["onTap"]["innertubeCommand"]));

        start = startIndex + length;
    }

    if (QString lastText = content.mid(start); !lastText.isEmpty())
        out.runs.append(InnertubeObjects::InnertubeRun(lastText));

    return out;
}

void WatchView::updateMetadata(const QString& videoId)
{
    try
    {
        auto endpoint = InnerTube::instance()->getBlocking<InnertubeEndpoints::UpdatedMetadata>(videoId);
        if (size_t superTitleIndex = ui->date->text().indexOf(" | "); superTitleIndex != -1)
            ui->date->setText(endpoint.response.dateText + ui->date->text().mid(superTitleIndex));
        else
            ui->date->setText(endpoint.response.dateText);

        ui->description->setText(StringUtils::innertubeStringToRichText(endpoint.response.description, false));
        ui->likeLabel->setText(qtTubeApp->settings().condensedCounts
                                   ? endpoint.response.likeCountEntity.likeCountIfIndifferent
                                   : endpoint.response.likeCountEntity.expandedLikeCountIfIndifferent);
        ui->titleLabel->setText(endpoint.response.title.text);
        ui->viewCount->setText(endpoint.response.viewCount.viewCount.text);

        if (qtTubeApp->settings().returnDislikes)
        {
            HttpReply* reply = Http::instance().get("https://returnyoutubedislikeapi.com/votes?videoId=" + videoId);
            connect(reply, &HttpReply::finished, this, &WatchView::setDislikes);
        }
    }
    catch (const InnertubeException& ie)
    {
        qDebug() << ie.message() << "Stream/premiere could have ended - killing update timer.";
        metadataUpdateTimer->deleteLater();
    }
}
