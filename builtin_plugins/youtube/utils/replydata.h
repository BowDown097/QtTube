#pragma once
#include "innertube/endpoints/browse/browsehome.h"
#include "qttube-plugin/objects/video.h"

QList<QtTube::PluginVideo> getHomeData(const InnertubeEndpoints::BrowseHome& endpoint);
