#pragma once
#include "innertube/responses/browse/historyresponse.h"
#include "innertube/responses/browse/homeresponse.h"
#include "innertube/responses/browse/subscriptionsresponse.h"
#include "innertube/responses/browse/trendingresponse.h"
#include "innertube/responses/misc/searchresponse.h"
#include "innertube/responses/notification/notificationmenuresponse.h"
#include "innertube/responses/video/nextresponse.h"
#include "innertube/responses/video/playerresponse.h"
#include "qttube-plugin/components/replytypes.h"

QtTube::BrowseData getHistoryData(const InnertubeEndpoints::HistoryResponse& response);
QtTube::BrowseData getHomeData(const InnertubeEndpoints::HomeResponse& response);
void getNextData(QtTube::VideoData& data, const InnertubeEndpoints::NextResponse& response);
QtTube::NotificationsData getNotificationsData(const InnertubeEndpoints::NotificationMenuResponse& response);
void getPlayerData(QtTube::VideoData& data, const InnertubeEndpoints::PlayerResponse& response);
QtTube::BrowseData getSearchData(const InnertubeEndpoints::SearchResponse& response);
QtTube::BrowseData getSubscriptionsData(const InnertubeEndpoints::SubscriptionsResponse& response);
QtTube::BrowseData getTrendingData(const InnertubeEndpoints::TrendingResponse& response);
