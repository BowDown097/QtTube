#pragma once
#include "innertube/responses/browse/historyresponse.h"
#include "innertube/responses/browse/homeresponse.h"
#include "innertube/responses/browse/subscriptionsresponse.h"
#include "innertube/responses/browse/trendingresponse.h"
#include "innertube/responses/notification/notificationmenuresponse.h"
#include "qttube-plugin/components/replytypes.h"

QtTube::BrowseData getHistoryData(const InnertubeEndpoints::HistoryResponse& response);
QtTube::BrowseData getHomeData(const InnertubeEndpoints::HomeResponse& response);
QtTube::NotificationsData getNotificationsData(const InnertubeEndpoints::NotificationMenuResponse& response);
QtTube::BrowseData getSubscriptionsData(const InnertubeEndpoints::SubscriptionsResponse& response);
QtTube::BrowseData getTrendingData(const InnertubeEndpoints::TrendingResponse& response);
