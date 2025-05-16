#pragma once
#include "innertube/endpoints/browse/browsehome.h"
#include "innertube/endpoints/browse/browsetrending.h"
#include "qttube-plugin/components/replytypes.h"

QtTube::HomeData getHomeData(const InnertubeEndpoints::BrowseHome& endpoint);
QtTube::TrendingData getTrendingData(const InnertubeEndpoints::BrowseTrending& endpoint);
