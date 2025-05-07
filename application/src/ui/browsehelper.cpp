#include "browsehelper.h"
#include "channelbrowser.h"
#include "mainwindow.h"
#include "protobuf/protobufcompiler.h"
#include "qttubeapplication.h"
#include <ranges>

using namespace InnertubeEndpoints;

BrowseHelper* BrowseHelper::instance()
{
    std::call_once(m_onceFlag, [] { m_instance = new BrowseHelper; });
    return m_instance;
}

void BrowseHelper::browseChannel(ContinuableListWidget* widget, int index, const InnertubeEndpoints::ChannelResponse& resp)
{
    QJsonValue tabRenderer = resp.contents["twoColumnBrowseResultsRenderer"]["tabs"][index]["tabRenderer"];
    if (!tabRenderer["selected"].toBool())
    {
        QString params = tabRenderer["endpoint"]["browseEndpoint"]["params"].toString();
        auto bc = InnerTube::instance()->getBlocking<BrowseChannel>(resp.metadata.externalId, "", params);
        tabRenderer = bc.response.contents["twoColumnBrowseResultsRenderer"]["tabs"][index]["tabRenderer"];
    }

    try
    {
        QString commandUrl = tabRenderer["endpoint"]["commandMetadata"]["webCommandMetadata"]["url"].toString();
        if (commandUrl.endsWith("/featured"))
            ChannelBrowser::setupHome(widget, tabRenderer);
        else if (commandUrl.endsWith("/videos"))
            ChannelBrowser::setupVideos(widget, tabRenderer);
        else if (commandUrl.endsWith("/shorts"))
            ChannelBrowser::setupShorts(widget, tabRenderer);
        else if (commandUrl.endsWith("/streams"))
            ChannelBrowser::setupLive(widget, tabRenderer);
        else if (commandUrl.endsWith("/membership"))
            ChannelBrowser::setupMembership(widget, tabRenderer);
        else if (commandUrl.endsWith("/community"))
            ChannelBrowser::setupCommunity(widget, tabRenderer);
        else
            ChannelBrowser::setupUnimplemented(widget);
    }
    catch (const InnertubeException& ie)
    {
        browseFailed("channel tab", nullptr, ie);
    }
}

void BrowseHelper::browseHistory(ContinuableListWidget* widget, const QString& query)
{
    if (!InnerTube::instance()->hasAuthenticated())
    {
        widget->addItem("Local history has not been implemented yet. You will need to log in.");
        return;
    }

    widget->setPopulatingFlag(true);
    auto reply = InnerTube::instance()->get<BrowseHistory>(query);
    connect(reply, &InnertubeReply<BrowseHistory>::exception, this,
        std::bind_front(&BrowseHelper::browseFailed, this, "history", widget));
    connect(reply, &InnertubeReply<BrowseHistory>::finished, this, [this, widget](const BrowseHistory& endpoint) {
        UIUtils::addRangeToList(widget, endpoint.response.videos);
        widget->continuationToken = endpoint.continuationToken;
        widget->setPopulatingFlag(false);
    });
}

void BrowseHelper::browseHome(ContinuableListWidget* widget)
{
    widget->setPopulatingFlag(true);

    // most clients will not serve you a home page unless if you are logged in or have searched for videos.
    // i don't like this. thankfully, there's a few clients that do still:
    // ANDROID_TESTSUITE, ANDROID_VR, IOS_UNPLUGGED, TVAPPLE, and TVHTML5
    // IOS_UNPLUGGED is the only one that works with tryCreate currently, so it will be used.
    if (InnerTube::instance()->hasAuthenticated())
    {
        auto reply = InnerTube::instance()->get<BrowseHome>();
        connect(reply, &InnertubeReply<BrowseHome>::exception, this,
            std::bind_front(&BrowseHelper::browseFailed, this, "home", widget));
        connect(reply, &InnertubeReply<BrowseHome>::finished, this, [this, widget](const BrowseHome& endpoint) {
            setupHome(widget, endpoint.response);
            widget->continuationToken = endpoint.continuationToken;
            widget->setPopulatingFlag(false);
        });
    }
    else
    {
        auto reply = InnerTube::instance()->getRaw<BrowseHome>({
            { "context", QJsonObject {
                { "client", QJsonObject {
                    { "clientName", static_cast<int>(InnertubeClient::ClientType::IOS_UNPLUGGED) },
                    { "clientVersion", InnertubeClient::getLatestVersion(InnertubeClient::ClientType::IOS_UNPLUGGED) }
                }}
            }}
        });

        connect(reply, &InnertubeReply<BrowseHome>::exception, this,
            std::bind_front(&BrowseHelper::browseFailed, this, "home", widget));
        connect(reply, &InnertubeReply<BrowseHome>::finishedRaw, this, [this, widget](const QJsonValue& data) {
            if (const auto endpoint = InnerTube::tryCreate<BrowseHome>(data))
            {
                setupHome(widget, endpoint->response);
                widget->continuationToken = endpoint->continuationToken;
            }
            else
            {
                browseFailed("home", widget, endpoint.error());
            }

            widget->setPopulatingFlag(false);
        });
    }
}

void BrowseHelper::browseNotificationMenu(ContinuableListWidget* widget)
{
    widget->setPopulatingFlag(true);
    auto reply = InnerTube::instance()->get<GetNotificationMenu>("NOTIFICATIONS_MENU_REQUEST_TYPE_INBOX");
    connect(reply, &InnertubeReply<GetNotificationMenu>::exception, this,
        std::bind_front(&BrowseHelper::browseFailed, this, "notification", widget));
    connect(reply, &InnertubeReply<GetNotificationMenu>::finished, this, [this, widget](const GetNotificationMenu& endpoint) {
        UIUtils::addRangeToList(widget, endpoint.response.notifications);
        widget->continuationToken = endpoint.continuationToken;
        MainWindow::topbar()->updateNotificationCount(0);
        widget->setPopulatingFlag(false);
    });
}

void BrowseHelper::browseSubscriptions(ContinuableListWidget* widget)
{
    if (!InnerTube::instance()->hasAuthenticated())
    {
        widget->addItem("You need to log in to view subscriptions.");
        return;
    }

    widget->setPopulatingFlag(true);
    auto reply = InnerTube::instance()->get<BrowseSubscriptions>();
    connect(reply, &InnertubeReply<BrowseSubscriptions>::exception, this,
        std::bind_front(&BrowseHelper::browseFailed, this, "subscriptions", widget));
    connect(reply, &InnertubeReply<BrowseSubscriptions>::finished, this, [this, widget](const BrowseSubscriptions& endpoint) {
        UIUtils::addRangeToList(widget, endpoint.response.videos);
        widget->continuationToken = endpoint.continuationToken;
        widget->setPopulatingFlag(false);
    });
}

void BrowseHelper::browseTrending(ContinuableListWidget* widget)
{
    widget->setPopulatingFlag(true);
    auto reply = InnerTube::instance()->get<BrowseTrending>();
    connect(reply, &InnertubeReply<BrowseTrending>::exception, this,
        std::bind_front(&BrowseHelper::browseFailed, this, "trending", widget));
    connect(reply, &InnertubeReply<BrowseTrending>::finished, this, [this, widget](const BrowseTrending& endpoint) {
        setupTrending(widget, endpoint.response);
        widget->setPopulatingFlag(false);
    });
}

void BrowseHelper::continueChannel(ContinuableListWidget* widget, const QJsonValue& contents)
{
    widget->continuationToken.clear();
    ChannelBrowser::continuation(widget, contents);
}

void BrowseHelper::search(ContinuableListWidget* widget, const QString& query,
                          int dateF, int typeF, int durF, int featF, int sort)
{
    QByteArray compiledParams;
    QVariantMap filter, params;
    if (sort != -1) params.insert("sort", sort);
    if (dateF != -1) filter.insert("uploadDate", dateF + 1);
    if (typeF != -1) filter.insert("type", typeF + 1);
    if (durF != -1) filter.insert("duration", durF + 1);
    if (featF != -1) filter.insert(featureMap[featF], true);
    if (!filter.isEmpty()) params.insert("filter", filter);

    if (!params.isEmpty())
        compiledParams = ProtobufCompiler::compileEncoded(params, searchMsgFields);

    widget->setPopulatingFlag(true);
    auto reply = InnerTube::instance()->get<Search>(query, "", compiledParams);
    connect(reply, &InnertubeReply<Search>::exception, this,
        std::bind_front(&BrowseHelper::browseFailed, this, "search", widget));
    connect(reply, &InnertubeReply<Search>::finished, this, [this, widget](const Search& endpoint) {
        widget->addItem(QStringLiteral("About %1 results").arg(QLocale::system().toString(endpoint.response.estimatedResults)));
        setupSearch(widget, endpoint.response);
        widget->continuationToken = endpoint.continuationToken;
        widget->setPopulatingFlag(false);
    });
}

void BrowseHelper::browseFailed(const QString& title, ContinuableListWidget* widget, const InnertubeException& ie)
{
    if (widget)
        widget->setPopulatingFlag(false);

    if (ie.severity() == InnertubeException::Severity::Normal)
        QMessageBox::critical(nullptr, "Failed to get " + title + " data", ie.message());
    else
        qWarning().nospace() << "Failed to get " << title << " data: " << ie.message();
}

void BrowseHelper::removeTrailingSeparator(QListWidget* list)
{
    if (QListWidgetItem* item = list->item(list->count()))
        if (QWidget* itemWidget = list->itemWidget(item))
            if (qobject_cast<QFrame*>(itemWidget))
                itemWidget->deleteLater();
}

// TODO: make reel shelf widget, and expandable list widget, replace applicable code

void BrowseHelper::setupHome(QListWidget* widget, const InnertubeEndpoints::HomeResponse& response)
{
    // non-authenticated users will be under the IOS_UNPLUGGED client,
    // which serves thumbnails in an odd aspect ratio.
    bool useThumbnailFromData = InnerTube::instance()->hasAuthenticated();

    for (const InnertubeEndpoints::HomeResponseItem& item : response.contents)
    {
        if (const auto* adSlot = std::get_if<InnertubeObjects::AdSlot>(&item))
        {
            UIUtils::addVideoToList(widget, *adSlot, useThumbnailFromData);
        }
        if (const auto* horizontalShelf = std::get_if<InnertubeObjects::HorizontalVideoShelf>(&item))
        {
            UIUtils::addShelfTitleToList(widget, horizontalShelf->title.text);

            for (const InnertubeObjects::Video& video : horizontalShelf->content.items)
            {
                UIUtils::addVideoToList(widget, video, useThumbnailFromData);
                QCoreApplication::processEvents();
            }

            UIUtils::addSeparatorToList(widget);
        }
        else if (const auto* lockup = std::get_if<InnertubeObjects::LockupViewModel>(&item))
        {
            UIUtils::addVideoToList(widget, *lockup, useThumbnailFromData);
        }
        else if (const auto* richShelf = std::get_if<InnertubeObjects::HomeRichShelf>(&item))
        {
            if (qtTubeApp->settings().hideShorts && std::ranges::any_of(richShelf->contents, [](const auto& item) {
                    return std::holds_alternative<InnertubeObjects::ShortsLockupViewModel>(item);
                }))
            {
                continue;
            }

            if (std::ranges::any_of(richShelf->contents, [](const auto& item) {
                    return std::holds_alternative<InnertubeObjects::MiniGameCardViewModel>(item);
                }))
            {
                continue;
            }

            UIUtils::addShelfTitleToList(widget, richShelf->title.text);

            for (const auto& itemVariant : richShelf->contents)
            {
                std::visit([widget, useThumbnailFromData](auto&& item) {
                    using ItemType = std::remove_cvref_t<decltype(item)>;
                    if constexpr (std::same_as<ItemType, InnertubeObjects::Post>)
                        UIUtils::addPostToList(widget, item);
                    else if constexpr (!std::same_as<ItemType, InnertubeObjects::MiniGameCardViewModel>)
                        UIUtils::addVideoToList(widget, item, useThumbnailFromData);
                }, itemVariant);

                QCoreApplication::processEvents();
            }

            UIUtils::addSeparatorToList(widget);
        }
        else if (const auto* video = std::get_if<InnertubeObjects::Video>(&item))
        {
            UIUtils::addVideoToList(widget, *video, useThumbnailFromData);
        }

        QCoreApplication::processEvents();
    }
}

void BrowseHelper::setupSearch(QListWidget* widget, const InnertubeEndpoints::SearchResponse& response)
{
    for (const InnertubeEndpoints::SearchResponseItem& item : response.contents)
    {
        if (const auto* channel = std::get_if<InnertubeObjects::Channel>(&item))
        {
            UIUtils::addChannelToList(widget, *channel);
            QCoreApplication::processEvents();
        }
        else if (const auto* reelShelf = std::get_if<InnertubeObjects::ReelShelf>(&item))
        {
            if (qtTubeApp->settings().hideSearchShelves || qtTubeApp->settings().hideShorts)
                continue;

            UIUtils::addSeparatorToList(widget);
            UIUtils::addShelfTitleToList(widget, reelShelf->title.text);
            UIUtils::addRangeToList(widget, reelShelf->items);
            UIUtils::addSeparatorToList(widget);
        }
        else if (const auto* verticalShelf = std::get_if<InnertubeObjects::VerticalVideoShelf>(&item))
        {
            if (qtTubeApp->settings().hideSearchShelves)
                continue;

            UIUtils::addSeparatorToList(widget);
            UIUtils::addShelfTitleToList(widget, verticalShelf->title.text);
            UIUtils::addRangeToList(widget,
                verticalShelf->content.items | std::views::take(verticalShelf->content.collapsedItemCount));
            UIUtils::addSeparatorToList(widget);
        }
        else if (const auto* video = std::get_if<InnertubeObjects::Video>(&item))
        {
            UIUtils::addVideoToList(widget, *video);
            QCoreApplication::processEvents();
        }
    }
}

void BrowseHelper::setupTrending(QListWidget* widget, const InnertubeEndpoints::TrendingResponse& response)
{
    for (const InnertubeEndpoints::TrendingResponseItem& item : response.contents)
    {
        if (const auto* horizontalShelf = std::get_if<InnertubeObjects::HorizontalVideoShelf>(&item))
        {
            UIUtils::addShelfTitleToList(widget, horizontalShelf->title.text);
            UIUtils::addRangeToList(widget, horizontalShelf->content.items);
            UIUtils::addSeparatorToList(widget);
        }
        else if (const auto* reelShelf = std::get_if<InnertubeObjects::ReelShelf>(&item))
        {
            if (qtTubeApp->settings().hideShorts)
                continue;

            UIUtils::addShelfTitleToList(widget, reelShelf->title.text);
            UIUtils::addRangeToList(widget, reelShelf->items);
            UIUtils::addSeparatorToList(widget);
        }
        else if (const auto* standardShelf = std::get_if<InnertubeObjects::StandardVideoShelf>(&item))
        {
            UIUtils::addShelfTitleToList(widget, standardShelf->title.text);
            UIUtils::addRangeToList(widget, standardShelf->content);
            UIUtils::addSeparatorToList(widget);
        }

        removeTrailingSeparator(widget);
    }
}
