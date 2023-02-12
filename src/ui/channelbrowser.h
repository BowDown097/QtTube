#ifndef CHANNELBROWSER_H
#define CHANNELBROWSER_H
#include "innertube/objects/channel/channel.h"
#include "innertube/objects/video/reel.h"
#include "innertube/objects/video/video.h"
#include "innertube/responses/browse/channelresponse.h"
#include <QListWidget>

class ChannelBrowser
{
public:
    static ChannelBrowser& instance() { static ChannelBrowser bh; return bh; }
    void setupAbout(QListWidget* channelTab, const QJsonValue& tabRenderer);
    void setupChannels(QListWidget* channelTab, const QJsonValue& tabRenderer);
    void setupHome(QListWidget* channelTab, const QJsonValue& tabRenderer, const InnertubeEndpoints::ChannelResponse& channelResp);
    void setupLive(QListWidget* channelTab, const QJsonValue& tabRenderer, const InnertubeEndpoints::ChannelResponse& channelResp);
    void setupShorts(QListWidget* channelTab, const QJsonValue& tabRenderer, const InnertubeEndpoints::ChannelResponse& channelResp);
    void setupUnimplemented(QListWidget* channelTab);
    void setupVideos(QListWidget* channelTab, const QJsonValue& tabRenderer, const InnertubeEndpoints::ChannelResponse& channelResp);
    void addChannelRendererFromChannel(QListWidget* list, const InnertubeObjects::Channel& channel);
    void addVideoRendererFromReel(QListWidget* list, const InnertubeObjects::Reel& reel);
    void addVideoRendererFromVideo(QListWidget* list, const InnertubeObjects::Video& video);
private:
    void addBoldLabel(QListWidget* list, const QString& text);
    void addLabel(QListWidget* list, const QString& text);
    void addSeparatorItem(QListWidget* list);
};

#endif // CHANNELBROWSER_H
