#pragma once
#include "ui/widgets/continuablelistwidget.h"

namespace ChannelBrowser
{
    void continuation(ContinuableListWidget* widget, const QJsonValue& contents);
    void setupCommunity(ContinuableListWidget* widget, const QJsonValue& renderer);
    void setupHome(ContinuableListWidget* widget, const QJsonValue& renderer);
    void setupLive(ContinuableListWidget* widget, const QJsonValue& renderer);
    void setupMembership(ContinuableListWidget* widget, const QJsonValue& renderer);
    void setupShorts(ContinuableListWidget* widget, const QJsonValue& renderer);
    void setupUnimplemented(ContinuableListWidget* widget);
    void setupVideos(ContinuableListWidget* widget, const QJsonValue& renderer);
};
