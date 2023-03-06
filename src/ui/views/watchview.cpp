#include "watchview.h"
#include "channelview.h"
#include "http.h"
#include "innertube.h"
#include "settingsstore.h"
#include "ui/forms/mainwindow.h"
#include "ui/uiutilities.h"
#include <QApplication>
#include <QDesktopServices>
#include <QMenu>
#include <QMessageBox>
#include <QResizeEvent>
#include <QScrollBar>

#ifdef USEMPV
#include "lib/media/mpv/mediampv.h"
#include <QRandomGenerator>
#endif

#ifdef QTTUBE_HAS_XSS
#include <X11/extensions/scrnsaver.h>
#ifdef QTTUBE_HAS_X11EXTRAS
#include <QtX11Extras/QX11Info>
#endif // X11Extras check
#endif // XScreenSaver check

#ifdef Q_OS_WIN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

WatchView* WatchView::instance()
{
    if (!m_watchView)
        m_watchView = new WatchView;
    return m_watchView;
}

void WatchView::goBack()
{
#ifdef USEMPV
    if (media)
    {
        media->stop();
        media->clearQueue();
        media->deleteLater();
        media = nullptr;
    }

    if (watchtimeTimer)
    {
        watchtimeTimer->stop();
        watchtimeTimer->deleteLater();
        watchtimeTimer = nullptr;
    }

    for (QAction* action : actions())
        removeAction(action);
#endif
    MainWindow::topbar()->alwaysShow = true;
    MainWindow::centralWidget()->setCurrentIndex(0);
    disconnect(MainWindow::topbar()->logo, &TubeLabel::clicked, this, &WatchView::goBack);

    UIUtilities::clearLayout(frame->layout());
    frame->layout()->deleteLater();
    toggleIdleSleep(false);

    if (metadataUpdateTimer)
    {
        metadataUpdateTimer->stop();
        metadataUpdateTimer->deleteLater();
        metadataUpdateTimer = nullptr;
    }
}

void WatchView::loadVideo(const QString& videoId, int progress)
{
    MainWindow::centralWidget()->setCurrentIndex(1);

    scrollArea = new QScrollArea(this);
    scrollArea->setFixedSize(size());
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setWidgetResizable(true);
    scrollArea->show();

    // it seems, for whatever reason, if i just use a QVBoxLayout as opposed to one wrapped in a frame,
    // it does not resize, meaning all the content is just squished.
    frame = new QFrame(scrollArea);
    frame->setLayout(new QVBoxLayout);
    frame->layout()->setContentsMargins(0, 0, 0, 0);
    frame->layout()->setSpacing(5);
    scrollArea->setWidget(frame);

    QSize playerSize = calcPlayerSize();

    InnertubeReply* next = InnerTube::instance().get<InnertubeEndpoints::Next>(videoId);
    connect(next, qOverload<InnertubeEndpoints::Next>(&InnertubeReply::finished), this, &WatchView::processNext);
    connect(next, &InnertubeReply::exception, this, &WatchView::loadFailed);

    InnertubeReply* player = InnerTube::instance().get<InnertubeEndpoints::Player>(videoId);
    connect(player, qOverload<InnertubeEndpoints::Player>(&InnertubeReply::finished), this, &WatchView::processPlayer);
    connect(player, &InnertubeReply::exception, this, &WatchView::loadFailed);

#ifdef USEMPV
    media = new MediaMPV;
    media->init();
    media->play("https://www.youtube.com/watch?v=" + videoId);
    media->seek(progress);
    media->setVolume(SettingsStore::instance().preferredVolume);
    media->videoWidget()->setFixedSize(playerSize);
    frame->layout()->addWidget(media->videoWidget());

    connect(media, &Media::error, this, [this](const QString& message) { QMessageBox::warning(this, "Media error", message); });
    connect(media, &Media::stateChanged, this, &WatchView::mediaStateChanged);
    connect(media, &Media::volumeChanged, this, &WatchView::volumeChanged);
#else
    wePlayer = new WebEnginePlayer(this);
    wePlayer->setAuthStore(InnerTube::instance().authStore());
    wePlayer->setContext(InnerTube::instance().context());
    wePlayer->setFixedSize(playerSize);
    wePlayer->play(videoId, progress);
    frame->layout()->addWidget(wePlayer);
#endif

    titleLabel = new TubeLabel(this);
    titleLabel->setFixedWidth(playerSize.width());
    titleLabel->setFont(QFont(qApp->font().toString(), qApp->font().pointSize() + 4));
    titleLabel->setWordWrap(true);
    frame->layout()->addWidget(titleLabel);

    { // begin primaryInfoHbox
        primaryInfoHbox = new QHBoxLayout(this);
        primaryInfoHbox->setContentsMargins(0, 0, 0, 0);

        channelIcon = new TubeLabel(this);
        channelIcon->setClickable(true, false);
        channelIcon->setMaximumSize(55, 48);
        channelIcon->setMinimumSize(55, 48);
        primaryInfoHbox->addWidget(channelIcon);

        { // begin primaryInfoVbox
            primaryInfoVbox = new QVBoxLayout(this);

            channelLabel = new ChannelLabel(this);
            primaryInfoVbox->addWidget(channelLabel);
            connect(channelLabel->text, &TubeLabel::customContextMenuRequested, this, &WatchView::showContextMenu);

            { // begin subscribeHbox
                subscribeHbox = new QHBoxLayout(this);
                subscribeHbox->setContentsMargins(0, 0, 0, 0);
                subscribeHbox->setSpacing(0);

                subscribeWidget = new SubscribeWidget(this);
                subscribeHbox->addWidget(subscribeWidget);

                subscribersLabel = new TubeLabel(this);
                subscribeHbox->addWidget(subscribersLabel);

                subscribeHbox->addStretch();

                primaryInfoVbox->addLayout(subscribeHbox);
            } // end subscribeHbox

            primaryInfoHbox->addLayout(primaryInfoVbox);
        } // end primaryInfoVbox

        primaryInfoHbox->addStretch();

        primaryInfoWrapper = new QWidget(this);
        primaryInfoWrapper->setFixedWidth(playerSize.width());
        primaryInfoWrapper->setLayout(primaryInfoHbox);
        frame->layout()->addWidget(primaryInfoWrapper);
    } // end primaryInfoHbox

    { // begin menuVbox
        menuVbox = new QVBoxLayout(this);
        menuVbox->setContentsMargins(0, 0, 20, 0);
        menuVbox->setSpacing(3);

        viewCount = new TubeLabel(this);
        viewCount->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        viewCount->setFont(QFont(qApp->font().toString(), qApp->font().pointSize() + 3));
        menuVbox->addWidget(viewCount);

        if (SettingsStore::instance().returnDislikes)
        {
            // i have to wrap the like bar for alignment to work... cringe!
            likeBarWrapper = new QHBoxLayout(this);
            likeBarWrapper->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

            likeBar = new QProgressBar(this);
            likeBar->setFixedSize(155, 2);
            likeBar->setStyleSheet(R"(
            QProgressBar {
                border-radius: 2px;
                background-color: #606060;
            }

            QProgressBar::chunk {
                background-color: #1879c6;
            }
            )");
            likeBar->setTextVisible(false);
            likeBar->setVisible(false);
            likeBarWrapper->addWidget(likeBar);

            menuVbox->addLayout(likeBarWrapper);
        }

        topLevelButtons = new QHBoxLayout(this);
        topLevelButtons->setContentsMargins(0, 3, 0, 0);
        topLevelButtons->setSpacing(0);
        menuVbox->addLayout(topLevelButtons);

        menuWrapper = new QWidget(this);
        menuWrapper->setFixedWidth(playerSize.width());
        menuWrapper->setLayout(menuVbox);
        frame->layout()->addWidget(menuWrapper);
    } // end menuVbox

    infoSpacer = new QSpacerItem(20, 20);
    frame->layout()->addItem(infoSpacer);

    date = new TubeLabel(this);
    date->setFont(QFont(qApp->font().toString(), -1, QFont::Bold));
    frame->layout()->addWidget(date);

    description = new TubeLabel(this);
    description->setFixedWidth(playerSize.width());
    description->setTextFormat(Qt::RichText);
    description->setWordWrap(true);
    UIUtilities::setMaximumLines(description, 3);
    frame->layout()->addWidget(description);
    connect(description, &TubeLabel::linkActivated, this, &WatchView::descriptionLinkActivated);

    showMoreLabel = new TubeLabel(this);
    showMoreLabel->setAlignment(Qt::AlignCenter);
    showMoreLabel->setClickable(true, false);
    showMoreLabel->setFixedWidth(playerSize.width());
    showMoreLabel->setStyleSheet("border-top: 1px solid " + qApp->palette().text().color().name());
    showMoreLabel->setText("SHOW MORE");
    frame->layout()->addWidget(showMoreLabel);
    connect(showMoreLabel, &TubeLabel::clicked, this, &WatchView::toggleShowMore);

    qobject_cast<QVBoxLayout*>(frame->layout())->addStretch(); // disable the layout from stretching on resize

    toggleIdleSleep(true);
    MainWindow::topbar()->setVisible(false);
    MainWindow::topbar()->alwaysShow = false;
    connect(MainWindow::topbar()->logo, &TubeLabel::clicked, this, &WatchView::goBack);
}

void WatchView::hotLoadVideo(const QString& videoId, int progress)
{
#ifdef USEMPV
    media->play("https://www.youtube.com/watch?v=" + videoId);
    media->seek(progress);
    watchtimeTimer->deleteLater();
#else
    wePlayer->play(videoId, progress);
#endif

    UIUtilities::clearLayout(topLevelButtons);
    disconnect(channelLabel->text, &TubeLabel::clicked, nullptr, nullptr);

    InnertubeReply* next = InnerTube::instance().get<InnertubeEndpoints::Next>(videoId);
    connect(next, qOverload<InnertubeEndpoints::Next>(&InnertubeReply::finished), this, &WatchView::processNext);
    connect(next, &InnertubeReply::exception, this, &WatchView::loadFailed);

    InnertubeReply* player = InnerTube::instance().get<InnertubeEndpoints::Player>(videoId);
    connect(player, qOverload<InnertubeEndpoints::Player>(&InnertubeReply::finished), this, &WatchView::processPlayer);
    connect(player, &InnertubeReply::exception, this, &WatchView::loadFailed);
}

void WatchView::processNext(const InnertubeEndpoints::Next& endpoint)
{
    InnertubeEndpoints::NextResponse nextResp = endpoint.response;

    channelId = nextResp.secondaryInfo.subscribeButton.channelId;
    channelLabel->setInfo(nextResp.secondaryInfo.owner.title.text, nextResp.secondaryInfo.owner.badges);

    connect(channelLabel->text, &TubeLabel::clicked, this, std::bind(&WatchView::navigateChannel, this, channelId));

    subscribersLabel->setStyleSheet(R"(
    border: 1px solid #333;
    font-size: 11px;
    line-height: 24px;
    padding: 0 6px 0 4.5px;
    border-radius: 2px;
    text-align: center;
    )");

    subscribeWidget->setSubscribeButton(nextResp.secondaryInfo.subscribeButton);
    setSubscriberCount(nextResp.secondaryInfo);

    viewCount->setText(nextResp.primaryInfo.viewCount.text);

    for (const InnertubeObjects::MenuFlexibleItem& fi : nextResp.primaryInfo.videoActions.flexibleItems)
    {
        if (fi.topLevelButton.iconType == "CONTENT_CUT" || fi.topLevelButton.iconType == "MONEY_HEART") // will almost definitely never be implemented
            continue;

        QString labelText = fi.topLevelButton.text.text;
        if (labelText == "Save") // to replicate Hitchhiker style
            labelText = "Add to";

        IconLabel* label = new IconLabel(
            fi.topLevelButton.iconType.toLower(),
            topLevelButtons->count() > 0 ? QMargins(15, 0, 0, 0) : QMargins(5, 0, 0, 0)
        );
        label->setText(labelText);
        topLevelButtons->addWidget(label);
    }

    IconLabel* shareLabel = new IconLabel("share", topLevelButtons->count() > 0 ? QMargins(15, 0, 0, 0) : QMargins(5, 0, 0, 0));
    shareLabel->setText("Share");
    topLevelButtons->addWidget(shareLabel);

    topLevelButtons->addStretch();

    likeLabel = new IconLabel("like", QMargins(0, 0, 15, 0));
    topLevelButtons->addWidget(likeLabel);
    connect(likeLabel, &IconLabel::clicked, this, std::bind(&WatchView::likeOrDislike, this, true, nextResp.primaryInfo.videoActions.likeButton));
    if (nextResp.primaryInfo.videoActions.likeButton.isToggled)
    {
        likeLabel->icon->setPixmap(QPixmap(":/like-toggled.png").scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        likeLabel->textLabel->setStyleSheet("color: #167ac6");
    }

    dislikeLabel = new IconLabel("dislike");
    topLevelButtons->addWidget(dislikeLabel);
    connect(dislikeLabel, &IconLabel::clicked, this, std::bind(&WatchView::likeOrDislike, this, false, nextResp.primaryInfo.videoActions.dislikeButton));
    if (nextResp.primaryInfo.videoActions.dislikeButton.isToggled)
    {
        dislikeLabel->icon->setPixmap(QPixmap(":/dislike-toggled.png").scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        dislikeLabel->textLabel->setStyleSheet("color: #167ac6");
    }

    QList<InnertubeObjects::GenericThumbnail> channelIcons = nextResp.secondaryInfo.owner.thumbnails;
    if (!channelIcons.isEmpty())
    {
        auto bestThumb = *std::find_if(channelIcons.cbegin(), channelIcons.cend(), [](const InnertubeObjects::GenericThumbnail& t)
        {
            return t.width >= 48;
        });
        HttpReply* reply = Http::instance().get(bestThumb.url);
        connect(reply, &HttpReply::finished, this, &WatchView::setChannelIcon);
    }

    if (SettingsStore::instance().returnDislikes)
    {
        HttpReply* reply = Http::instance().get("https://returnyoutubedislikeapi.com/votes?videoId=" + nextResp.videoId);
        connect(reply, &HttpReply::finished, this, [this](const HttpReply& reply) {
            QJsonDocument doc = QJsonDocument::fromJson(reply.body());
            int dislikes = doc["dislikes"].toInt();
            int likes = doc["likes"].toInt();
            if (likes != 0 || dislikes != 0)
            {
                likeBar->setMaximum(likes + dislikes);
                likeBar->setValue(likes);
            }
            likeBar->setVisible(true);

            dislikeLabel->setText(QLocale::system().toString(dislikes));
            likeLabel->setText(QLocale::system().toString(likes));
        });
    }
    else
    {
        likeLabel->setText(nextResp.primaryInfo.videoActions.likeButton.defaultText.text);
    }

    QString dateText = nextResp.primaryInfo.dateText.text;
    if (!dateText.startsWith("Premier") && !dateText.startsWith("Stream") && !dateText.startsWith("Start") && !dateText.startsWith("Sched"))
        dateText = "Published on " + dateText;
    date->setText(dateText);

    description->setText(generateFormattedDescription(nextResp.secondaryInfo.description));
}

void WatchView::processPlayer(const InnertubeEndpoints::Player& endpoint)
{
    InnertubeEndpoints::PlayerResponse playerResp = endpoint.response;
    titleLabel->setText(playerResp.videoDetails.title);

#ifdef USEMPV
    if (SettingsStore::instance().playbackTracking)
        reportPlayback(playerResp);

    if (SettingsStore::instance().watchtimeTracking)
    {
        watchtimeTimer = new QTimer;
        watchtimeTimer->setInterval(5000);
        connect(watchtimeTimer, &QTimer::timeout, this, std::bind(&WatchView::reportWatchtime, this, playerResp, media->position));
        watchtimeTimer->start();
    }
#else
    wePlayer->setPlayerResponse(playerResp);
#endif

    if (playerResp.videoDetails.isLive)
    {
        metadataUpdateTimer = new QTimer;
        metadataUpdateTimer->setInterval(60000);
        connect(metadataUpdateTimer, &QTimer::timeout, this, [playerResp, this]
        {
            auto updatedMetadata = InnerTube::instance().getBlocking<InnertubeEndpoints::UpdatedMetadata>(playerResp.videoDetails.videoId);
            updateMetadata(updatedMetadata.response);
        });
        metadataUpdateTimer->start();
    }
}

void WatchView::resizeEvent(QResizeEvent* event)
{
    if (!primaryInfoWrapper || !event->oldSize().isValid()) return;

    QSize playerSize = calcPlayerSize();
    description->setFixedWidth(playerSize.width());
    menuWrapper->setFixedWidth(playerSize.width());
    primaryInfoWrapper->setFixedWidth(playerSize.width());
    showMoreLabel->setFixedWidth(playerSize.width());
    titleLabel->setFixedWidth(playerSize.width());

#ifdef USEMPV
    media->videoWidget()->setFixedSize(playerSize);
#else
    wePlayer->setFixedSize(playerSize);
#endif

    scrollArea->setFixedSize(event->size());
}

QSize WatchView::calcPlayerSize()
{
    int playerWidth = width();
    int playerHeight = playerWidth * 9/16;

    if (playerHeight > height() - 150)
    {
        playerHeight = height() - 150;
        playerWidth = playerHeight * 16/9;
    }

    return QSize(playerWidth, playerHeight);
}

void WatchView::copyChannelUrl()
{
    UIUtilities::copyToClipboard("https://www.youtube.com/channel/" + channelId);
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
        navigateChannel(funnyPath.left(funnyPath.indexOf('/')));
    }
    else if (url.startsWith("/watch"))
    {
        scrollArea->verticalScrollBar()->setValue(0);
        QUrlQuery query(qUrl);
        if (query.queryItemValue("continuePlayback") == "1")
        {
            int progress = query.queryItemValue("t").replace("s", "").toInt();
#ifdef USEMPV
    media->seek(progress);
#else
    wePlayer->seek(progress);
#endif
        }
        else
        {
            hotLoadVideo(query.queryItemValue("v"), query.queryItemValue("t").toInt());
            toggleShowMore();
        }
    }
    else
    {
        qDebug() << "Ran into unsupported description link:" << url;
    }
}

QString WatchView::generateFormattedDescription(const InnertubeObjects::InnertubeString& description)
{
    QString descriptionText;

    for (const InnertubeObjects::InnertubeRun& run : description.runs)
    {
        if (!run.navigationEndpoint.isNull() && !run.navigationEndpoint.isUndefined())
        {
            const QJsonObject navigationEndpoint = run.navigationEndpoint.toObject();
            QString href;
            QString runText = run.text;

            if (navigationEndpoint.contains("urlEndpoint"))
            {
                QUrl url(navigationEndpoint["urlEndpoint"]["url"].toString());
                QUrlQuery query(url);
                if (query.hasQueryItem("q"))
                {
                    href = QUrl::fromPercentEncoding(query.queryItemValue("q").toUtf8());
                }
                else if (url.host() == "www.youtube.com" && url.path().startsWith("/channel"))
                {
                    href = url.path();
                    runText = url.toString().left(37) + "...";
                }
                else
                {
                    href = url.toString();
                }
            }
            else
            {
                href = navigationEndpoint["commandMetadata"]["webCommandMetadata"]["url"].toString();
                if (navigationEndpoint.contains("watchEndpoint"))
                    href += "&continuePlayback=" + QString::number(navigationEndpoint["watchEndpoint"]["continuePlayback"].toBool());
            }

            descriptionText += QStringLiteral("<a href=\"%1\">%2</a>").arg(href, runText);
        }
        else
        {
            descriptionText += run.text;
        }
    }

    return descriptionText.replace("\n", "<br>");
}

void WatchView::likeOrDislike(bool like, const InnertubeObjects::ToggleButton& toggleButton)
{
    IconLabel* senderLabel = qobject_cast<IconLabel*>(sender());

    bool textIsNumber;
    int count = QLocale::system().toInt(senderLabel->textLabel->text(), &textIsNumber);

    if (senderLabel->textLabel->styleSheet().isEmpty()) // if untoggled
    {
        if (like)
            senderLabel->icon->setPixmap(QPixmap(":/like-toggled.png").scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        else
            senderLabel->icon->setPixmap(QPixmap(":/dislike-toggled.png").scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

        senderLabel->textLabel->setStyleSheet("color: #167ac6");
        if (textIsNumber)
            senderLabel->textLabel->setText(QLocale::system().toString(count + 1));

        const QJsonArray defaultCommands = toggleButton.defaultServiceEndpoint["commandExecutorCommand"]["commands"].toArray();
        QJsonValue defaultCommand = *std::find_if(defaultCommands.begin(), defaultCommands.end(),
            [](const QJsonValue& v) { return v.toObject().contains("commandMetadata"); });
        InnerTube::instance().like(defaultCommand["likeEndpoint"], like);
    }
    else
    {
        bool preferDark = qApp->palette().alternateBase().color().lightness() < 60;
        if (like)
            senderLabel->icon->setPixmap(QPixmap(preferDark ? ":/like-light.png" : ":/like.png").scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        else
            senderLabel->icon->setPixmap(QPixmap(preferDark ? ":/dislike-light.png" : ":/dislike.png").scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

        senderLabel->textLabel->setStyleSheet(QString());
        if (textIsNumber)
            senderLabel->textLabel->setText(QLocale::system().toString(count - 1));

        InnerTube::instance().like(toggleButton.toggledServiceEndpoint["likeEndpoint"], like);
    }
}

void WatchView::loadFailed(const InnertubeException& ie)
{
    QMessageBox::critical(this, "Failed to load video", ie.message());
    goBack();
}

void WatchView::navigateChannel(const QString& inChannelId)
{
    disconnect(MainWindow::topbar()->logo, &TubeLabel::clicked, this, &WatchView::goBack);
    toggleIdleSleep(false);

    try
    {
        ChannelView::instance()->loadChannel(inChannelId);
    }
    catch (const InnertubeException& ie)
    {
        QMessageBox::critical(nullptr, "Failed to load channel", ie.message());
    }

    UIUtilities::clearLayout(frame->layout());
    frame->layout()->deleteLater();
}

void WatchView::setChannelIcon(const HttpReply& reply)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    channelIcon->setPixmap(pixmap.scaled(48, 48, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void WatchView::setSubscriberCount(const InnertubeObjects::VideoSecondaryInfo& secondaryInfo)
{
    QString subscriberCountText = secondaryInfo.owner.subscriberCountText.text;
    if (!SettingsStore::instance().fullSubs)
    {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        subscribersLabel->setText(subscriberCountText.first(subscriberCountText.lastIndexOf(" ")));
#else
        subscribersLabel->setText(subscriberCountText.left(subscriberCountText.lastIndexOf(" ")));
#endif
        subscribersLabel->adjustSize();
        return;
    }

    Http http;
    http.setReadTimeout(2000);
    http.setMaxRetries(5);

    HttpReply* reply = http.get(QUrl("https://api.socialcounts.org/youtube-live-subscriber-count/" + secondaryInfo.subscribeButton.channelId));
    connect(reply, &HttpReply::error, this, [this, subscriberCountText] {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        subscribersLabel->setText(subscriberCountText.first(subscriberCountText.lastIndexOf(" ")));
#else
        subscribersLabel->setText(subscriberCountText.left(subscriberCountText.lastIndexOf(" ")));
#endif
        subscribersLabel->adjustSize();
    });
    connect(reply, &HttpReply::finished, this, [this](const HttpReply& reply) {
        int subs = QJsonDocument::fromJson(reply.body())["est_sub"].toInt();
        subscribersLabel->setText(QLocale::system().toString(subs));
        subscribersLabel->adjustSize();
    });
}

void WatchView::showContextMenu(const QPoint& pos)
{
    QMenu* menu = new QMenu(this);

    QAction* copyUrlAction = new QAction("Copy channel page URL", this);
    connect(copyUrlAction, &QAction::triggered, this, &WatchView::copyChannelUrl);

    menu->addAction(copyUrlAction);
    menu->popup(channelLabel->text->mapToGlobal(pos));
}

void WatchView::toggleIdleSleep(bool toggle)
{
#if defined(Q_OS_UNIX) && !defined(__APPLE__) && !defined(__MACH__)
    #ifdef QTTUBE_HAS_XSS
        if (qApp->platformName() != "xcb")
        {
            qDebug() << "Failed to toggle idle sleep: Can only toggle sleep on X11 on Unix systems. Screen may sleep while watching videos.";
            return;
        }

        #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            Display* display = qApp->nativeInterface<QNativeInterface::QX11Application>()->display();
        #elif defined(QTTUBE_HAS_X11EXTRAS)
            Display* display = QX11Info::display();
        #else
            Display* display = XOpenDisplay(NULL); // last resort!
        #endif // X11 display getter

        if (!display)
        {
            qDebug() << "Failed to toggle idle sleep: Failed to get X11 display";
            return;
        }

        int event, error, major, minor;
        if (XScreenSaverQueryExtension(display, &event, &error) != 1)
        {
            qDebug() << "Failed to toggle idle sleep: XScreenSaverQueryExtension failed";
            return;
        }
        if (XScreenSaverQueryVersion(display, &major, &minor) != 1 || major < 1 || (major == 1 && minor < 1))
        {
            qDebug() << "Failed to toggle idle sleep: XScreenSaverQueryVersion failed or definitely returned the wrong version";
            return;
        }

        XScreenSaverSuspend(display, toggle);
    #else
        qDebug() << "Failed to toggle idle sleep: XScreenSaver support is not enabled in this build";
        Q_UNUSED(toggle);
    #endif // XScreenSaver check
#elif defined(Q_OS_WIN)
    if (SetThreadExecutionState(toggle ? ES_DISPLAY_REQUIRED | ES_CONTINUOUS | ES_SYSTEM_REQUIRED : ES_CONTINUOUS) == NULL)
        qDebug() << "Failed to toggle idle sleep: SetThreadExecutionState failed";
#elif defined(Q_OS_MACOS)
    if (!toggle && sleepAssert)
    {
        IOPMAssertionRelease(sleepAssert);
        return;
    }

    CFStringRef reason = CFSTR("QtTube video playing");
    IOReturn success = IOPMAssertionCreateWithName(kIOPMAssertionTypeNoDisplaySleep, kIOPMAssertionLevelOn, reason, &sleepAssert);
    if (success != kIOReturnSuccess)
        qDebug() << "Failed to toggle idle sleep: Creating IOPM assertion failed";
#else
    qDebug() << "Failed to toggle idle sleep: Unsupported OS";
    Q_UNUSED(toggle);
#endif // OS checks
}

void WatchView::toggleShowMore()
{
    if (showMoreLabel->text() == "SHOW MORE")
    {
        description->setMaximumHeight(QWIDGETSIZE_MAX);
        showMoreLabel->setText("SHOW LESS");
    }
    else
    {
        UIUtilities::setMaximumLines(description, 3);
        showMoreLabel->setText("SHOW MORE");
    }
}

void WatchView::updateMetadata(const InnertubeEndpoints::UpdatedMetadataResponse& resp)
{
    description->setText(generateFormattedDescription(resp.description));
    titleLabel->setText(resp.title.text);
    viewCount->setText(resp.viewCount);

    QString dateText = resp.dateText;
    if (!dateText.startsWith("Premier") && !dateText.startsWith("Stream") && !dateText.startsWith("Start") && !dateText.startsWith("Sched"))
        dateText = "Published on " + dateText;
    date->setText(dateText);

    if (SettingsStore::instance().returnDislikes)
    {
        HttpReply* reply = Http::instance().get("https://returnyoutubedislikeapi.com/votes?videoId=" + resp.videoId);
        connect(reply, &HttpReply::finished, this, [this](const HttpReply& reply) {
            QJsonDocument doc = QJsonDocument::fromJson(reply.body());
            int dislikes = doc["dislikes"].toInt();
            int likes = doc["likes"].toInt();
            if (likes != 0 || dislikes != 0)
            {
                likeBar->setMaximum(likes + dislikes);
                likeBar->setValue(likes);
            }
            likeBar->setVisible(true);

            dislikeLabel->setText(QLocale::system().toString(dislikes));
            likeLabel->setText(QLocale::system().toString(likes));
        });
    }
    else
    {
        likeLabel->setText(resp.likeDefaultText);
    }
}

#ifdef USEMPV // MPV backend exclusive methods
void WatchView::mediaStateChanged(Media::State state)
{
    if (state == Media::ErrorState)
        QMessageBox::critical(this, "Media error", media->errorString());
}

void WatchView::volumeChanged(double volume)
{
    Q_UNUSED(volume);
    if (media->volumeMuted())
        media->setVolumeMuted(false);
}

QString WatchView::getCpn()
{
    QString out;
    constexpr std::string_view chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    for (int i = 0; i < 16; i++)
        out += chars[QRandomGenerator::global()->bounded((int)chars.size())];
    return out;
}

void WatchView::reportPlayback(const InnertubeEndpoints::PlayerResponse& playerResp)
{
    InnertubeClient itc = InnerTube::instance().context()->client;

    QUrlQuery playbackQuery(QUrl(playerResp.playbackTracking.videostatsPlaybackUrl));
    QUrl outPlaybackUrl("https://www.youtube.com/api/stats/playback");
    QUrlQuery outPlaybackQuery;

    QList<QPair<QString, QString>> map =
    {
        { "ns", "yt" },
        { "el", "detailpage" },
        { "cpn", getCpn() },
        { "ver", "2" },
        { "fmt", "243" },
        { "fs", "0" },
        { "rt", QString::number(QRandomGenerator::global()->bounded(191) + 10) },
        { "euri", "" },
        { "lact", QString::number(QRandomGenerator::global()->bounded(7001) + 1000) },
        { "cl", playbackQuery.queryItemValue("cl") },
        { "mos", "0" },
        { "volume", "100" },
        { "cbr", itc.browserName },
        { "cbrver", itc.browserVersion },
        { "c", itc.clientName },
        { "cver", itc.clientVersion },
        { "cplayer", "UNIPLAYER" },
        { "cos", itc.osName },
        { "cosver", itc.osVersion },
        { "cplatform", itc.platform },
        { "hl", itc.hl + "_" + itc.gl },
        { "cr", itc.gl },
        { "uga", playbackQuery.queryItemValue("uga") },
        { "len", playbackQuery.queryItemValue("len") },
        { "fexp", playbackQuery.queryItemValue("fexp") },
        { "rtn", "4" },
        { "afmt", "251" },
        { "muted", "0" },
        { "docid", playbackQuery.queryItemValue("docid") },
        { "ei", playbackQuery.queryItemValue("ei") },
        { "plid", playbackQuery.queryItemValue("plid") },
        { "sdetail", playbackQuery.queryItemValue("sdetail") },
        { "of", playbackQuery.queryItemValue("of") },
        { "vm", playbackQuery.queryItemValue("vm") }
    };

    outPlaybackQuery.setQueryItems(map);
    outPlaybackUrl.setQuery(outPlaybackQuery);
    Http::instance().get(outPlaybackUrl);
}

void WatchView::reportWatchtime(const InnertubeEndpoints::PlayerResponse& playerResp, long long position)
{
    InnertubeClient itc = InnerTube::instance().context()->client;

    QUrlQuery watchtimeQuery(QUrl(playerResp.playbackTracking.videostatsWatchtimeUrl));
    QUrl outWatchtimeUrl("https://www.youtube.com/api/stats/watchtime");
    QUrlQuery outWatchtimeQuery;
    QString rt = QString::number(QRandomGenerator::global()->bounded(191) + 10);
    QString posStr = QString::number(position);

    QList<QPair<QString, QString>> map =
    {
        { "ns", "yt" },
        { "el", "detailpage" },
        { "cpn", getCpn() },
        { "ver", "2" },
        { "fmt", "243" },
        { "fs", "0" },
        { "rt", rt },
        { "euri", "" },
        { "lact", QString::number(QRandomGenerator::global()->bounded(7001) + 1000) },
        { "cl", watchtimeQuery.queryItemValue("cl") },
        { "state", "playing" },
        { "volume", "100" },
        { "subscribed", watchtimeQuery.queryItemValue("subscribed") },
        { "cbr", itc.browserName },
        { "cbrver", itc.browserVersion },
        { "c", itc.clientName },
        { "cver", itc.clientVersion },
        { "cplayer", "UNIPLAYER" },
        { "cos", itc.osName },
        { "cosver", itc.osVersion },
        { "cplatform", itc.platform },
        { "hl", itc.hl + "_" + itc.gl },
        { "cr", itc.gl },
        { "uga", watchtimeQuery.queryItemValue("uga") },
        { "len", watchtimeQuery.queryItemValue("len") },
        { "afmt", "251" },
        { "idpj", "-1" },
        { "ldpj", "-10" },
        { "rti", rt },
        { "st", posStr },
        { "et", posStr },
        { "muted", "0" },
        { "docid", watchtimeQuery.queryItemValue("docid") },
        { "ei", watchtimeQuery.queryItemValue("ei") },
        { "plid", watchtimeQuery.queryItemValue("plid") },
        { "sdetail", watchtimeQuery.queryItemValue("sdetail") },
        { "of", watchtimeQuery.queryItemValue("of") },
        { "vm", watchtimeQuery.queryItemValue("vm") }
    };

    outWatchtimeQuery.setQueryItems(map);
    outWatchtimeUrl.setQuery(outWatchtimeQuery);
    Http::instance().get(outWatchtimeUrl);
}
#endif // MPV backend exclusive methods
