#ifndef CHANNELBROWSER_H
#define CHANNELBROWSER_H
#include "innertube/responses/browse/channelresponse.h"
#include <QListWidget>

class ChannelBrowser
{
public:
    static void setupAbout(QListWidget* channelTab, const QJsonValue& tabRenderer);
    static void setupChannels(QListWidget* channelTab, const QJsonValue& tabRenderer);
    static void setupHome(QListWidget* channelTab, const QJsonValue& tabRenderer, const InnertubeEndpoints::ChannelResponse& channelResp);
    static void setupLive(QListWidget* channelTab, const QJsonValue& tabRenderer, const InnertubeEndpoints::ChannelResponse& channelResp);
    static void setupMembership(QListWidget* channelTab, const QJsonValue& tabRenderer);
    static void setupShorts(QListWidget* channelTab, const QJsonValue& tabRenderer, const InnertubeEndpoints::ChannelResponse& channelResp);
    static void setupUnimplemented(QListWidget* channelTab);
    static void setupVideos(QListWidget* channelTab, const QJsonValue& tabRenderer, const InnertubeEndpoints::ChannelResponse& channelResp);
};

#endif // CHANNELBROWSER_H
