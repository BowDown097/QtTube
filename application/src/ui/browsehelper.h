#pragma once
#include "innertube.h"
#include "qttube-plugin/components/replytypes.h"
#include "utils/uiutils.h"
#include "ui/widgets/continuablelistwidget.h"
#include <QMessageBox>
#include <QScrollBar>

struct PluginData;
class QHBoxLayout;

class BrowseHelper : public QObject
{
    Q_OBJECT
public:
    static BrowseHelper* instance() { static BrowseHelper _instance; return &_instance; }
    explicit BrowseHelper(QObject* parent = nullptr) : QObject(parent) {}

    void browseChannel(ContinuableListWidget* widget, int index, const InnertubeEndpoints::ChannelResponse& resp);
    void browseHistory(ContinuableListWidget* widget, const QString& query = "");
    void browseHome(ContinuableListWidget* widget);
    void browseNotificationMenu(ContinuableListWidget* widget);
    void browseSubscriptions(ContinuableListWidget* widget);
    void browseTrending(ContinuableListWidget* widget);
    void continueChannel(ContinuableListWidget* widget, const QJsonValue& contents);
    void search(ContinuableListWidget* widget, QHBoxLayout* additionalWidgets, const QString& query);

    template<EndpointWithData E>
    void continuation(ContinuableListWidget* widget, const QString& data = "", int threshold = 10)
    {
        if (widget->continuationToken.isEmpty())
            return;

        if constexpr (std::same_as<E, InnertubeEndpoints::BrowseHome>)
        {
            browseHome(widget);
        }
        else if constexpr (std::same_as<E, InnertubeEndpoints::BrowseSubscriptions>)
        {
            browseSubscriptions(widget);
        }
        else if constexpr (std::same_as<E, InnertubeEndpoints::GetNotificationMenu>)
        {
            browseNotificationMenu(widget);
        }
        else if constexpr (std::same_as<E, InnertubeEndpoints::Search>)
        {
            search(widget, nullptr, data);
        }
        else
        {
            widget->setPopulatingFlag(true);

            try
            {
                E newData = InnerTube::instance()->getBlocking<E>(data, widget->continuationToken);
                if constexpr (std::same_as<E, InnertubeEndpoints::BrowseChannel>)
                    continueChannel(widget, newData.response.contents);
                else
                    UIUtils::addRangeToList(widget, newData.response.videos);

                // continuationToken is added by ChannelBrowser::continuation() for channels
                if constexpr (!std::same_as<E, InnertubeEndpoints::BrowseChannel>)
                    widget->continuationToken = newData.continuationToken;
            }
            catch (const InnertubeException& ie)
            {
                QMessageBox::critical(nullptr, "Failed to get continuation browsing info", ie.message());
            }

            widget->setPopulatingFlag(false);
        }
    }
private slots:
    void browseFailedInnertube(const QString& title, ContinuableListWidget* widget, const InnertubeException& ex);
    void browseFailedPlugin(const QString& title, ContinuableListWidget* widget, const QtTube::PluginException& ex);
private:
    QList<std::pair<QString, int>> getActiveFilters(QHBoxLayout* additionalWidgets);
    void setupBrowse(ContinuableListWidget* widget, QtTube::BrowseReply* reply, const QtTube::BrowseData& data);
    void setupNotifications(
        ContinuableListWidget* widget, QtTube::NotificationsReply* reply, const QtTube::NotificationsData& data);
    void setupSearch(
        ContinuableListWidget* widget, QHBoxLayout* additionalWidgets, const QString& query,
        const PluginData* plugin, QtTube::BrowseReply* reply, const QtTube::BrowseData& data);
};
