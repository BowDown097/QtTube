#ifndef CHANNELBROWSER_H
#define CHANNELBROWSER_H
#include <QListWidget>

namespace InnertubeEndpoints { class ChannelResponse; }

class ChannelBrowser
{
public:
    static void setupAbout(QListWidget* widget, const QJsonValue& renderer);
    static void setupChannels(QListWidget* widget, const QJsonValue& renderer);
    static void setupCommunity(QListWidget* widget, const QJsonValue& renderer);
    static void setupHome(QListWidget* widget, const QJsonValue& renderer, const InnertubeEndpoints::ChannelResponse& resp);
    static void setupLive(QListWidget* widget, const QJsonValue& renderer, const InnertubeEndpoints::ChannelResponse& resp);
    static void setupMembership(QListWidget* widget, const QJsonValue& renderer);
    static void setupShorts(QListWidget* widget, const QJsonValue& renderer, const InnertubeEndpoints::ChannelResponse& resp);
    static void setupUnimplemented(QListWidget* widget);
    static void setupVideos(QListWidget* widget, const QJsonValue& renderer, const InnertubeEndpoints::ChannelResponse& resp);
};

#endif // CHANNELBROWSER_H
