#pragma once
#include <QListWidget>

namespace InnertubeEndpoints { struct ChannelResponse; }

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
