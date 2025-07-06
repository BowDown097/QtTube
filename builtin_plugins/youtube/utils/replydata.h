#pragma once
#include "innertube/endpoints/live_chat/getlivechat.h"
#include "innertube/endpoints/live_chat/getlivechatreplay.h"
#include "innertube/responses/browse/channelresponse.h"
#include "innertube/responses/browse/historyresponse.h"
#include "innertube/responses/browse/homeresponse.h"
#include "innertube/responses/browse/subscriptionsresponse.h"
#include "innertube/responses/browse/trendingresponse.h"
#include "innertube/responses/misc/searchresponse.h"
#include "innertube/responses/notification/notificationmenuresponse.h"
#include "innertube/responses/video/nextresponse.h"
#include "innertube/responses/video/playerresponse.h"
#include "qttube-plugin/components/replytypes/channeldata.h"
#include "qttube-plugin/components/replytypes/simpledatatypes.h"
#include "qttube-plugin/components/replytypes/videodata.h"
#include "qttube-plugin/objects/livechat/livechat.h"
#include "qttube-plugin/objects/livechat/livechatreplay.h"

std::pair<std::any, QtTubePlugin::ChannelData> getChannelData(const InnertubeEndpoints::ChannelResponse& response);
QtTubePlugin::BrowseData getHistoryData(const InnertubeEndpoints::HistoryResponse& response);
QtTubePlugin::BrowseData getHomeData(const InnertubeEndpoints::HomeResponse& response);
QtTubePlugin::LiveChat getLiveChatData(const InnertubeEndpoints::GetLiveChat& endpoint, QJsonValue& sendEndpoint, int& sentMessages);
QtTubePlugin::LiveChatReplay getLiveChatReplayData(const InnertubeEndpoints::GetLiveChatReplay& endpoint);
void getNextData(QtTubePlugin::VideoData& data, const InnertubeEndpoints::NextResponse& response);
QtTubePlugin::NotificationsData getNotificationsData(const InnertubeEndpoints::NotificationMenuResponse& response);
void getPlayerData(QtTubePlugin::VideoData& data, const InnertubeEndpoints::PlayerResponse& response);
QtTubePlugin::BrowseData getSearchData(const InnertubeEndpoints::SearchResponse& response);
QtTubePlugin::BrowseData getSubscriptionsData(const InnertubeEndpoints::SubscriptionsResponse& response);
QtTubePlugin::BrowseData getTrendingData(const InnertubeEndpoints::TrendingResponse& response);
