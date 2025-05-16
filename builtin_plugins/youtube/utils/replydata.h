#pragma once
#include "innertube/endpoints/browse/browsehome.h"
#include "innertube/endpoints/browse/browsesubscriptions.h"
#include "innertube/endpoints/browse/browsetrending.h"
#include "qttube-plugin/components/replytypes.h"

QtTube::BrowseData getHomeData(const InnertubeEndpoints::BrowseHome& endpoint);
QtTube::BrowseData getSubscriptionsData(const InnertubeEndpoints::BrowseSubscriptions& endpoint);
QtTube::BrowseData getTrendingData(const InnertubeEndpoints::BrowseTrending& endpoint);
