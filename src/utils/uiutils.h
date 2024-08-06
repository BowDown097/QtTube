#pragma once
#include <initializer_list>
#include <QCoreApplication>
#include <QWidget>

namespace InnertubeObjects
{
struct BackstagePost;
struct Channel;
struct Notification;
struct Reel;
struct Video;
}

class QLabel;
class QLayout;
class QListWidget;
class QListWidgetItem;
class QTabWidget;
class VideoRenderer;

class UIUtils
{
public:
    static inline QString defaultStyle;

    static void addBackstagePostToList(QListWidget* list, const InnertubeObjects::BackstagePost& post);
    static void addBoldLabelToList(QListWidget* list, const QString& text);
    static void addChannelRendererToList(QListWidget* list, const InnertubeObjects::Channel& channel);
    static void addNotificationToList(QListWidget* list, const InnertubeObjects::Notification& notification);
    static void addSeparatorToList(QListWidget* list);
    static void addShelfTitleToList(QListWidget* list, const QJsonValue& shelf);
    static void addShelfTitleToList(QListWidget* list, const QString& title);
    static void addVideoRendererToList(QListWidget* list, const InnertubeObjects::Reel& reel);
    static void addVideoRendererToList(QListWidget* list, const InnertubeObjects::Video& video);
    static QListWidgetItem* addWidgetToList(QListWidget* list, QWidget* widget);
    static void addWrappedLabelToList(QListWidget* list, const QString& text);
    static void clearLayout(QLayout* layout);
    static void copyToClipboard(const QString& text);
    static QIcon iconThemed(const QString& name, const QPalette& pal = QPalette());
    static QPixmap pixmapRounded(const QPixmap& pixmap, double xRadius, double yRadius);
    static QPixmap pixmapThemed(const QString& name, bool fromQIcon = false, const QSize& size = QSize(), const QPalette& pal = QPalette());
    static bool preferDark(const QPalette& pal = QPalette());
    static QString resolveThemedIconName(const QString& name, const QPalette& pal = QPalette());
    static void setAppStyle(const QString& styleName, bool dark);
    static void setMaximumLines(QWidget* widget, int lines);
    static void setTabsEnabled(QTabWidget* widget, bool enabled, std::initializer_list<int> indexes);
    static void setThumbnail(QLabel* label, const QJsonArray& thumbsArr, bool getBest = false);

    static void addRangeToList(QListWidget* list, std::ranges::range auto&& range)
    {
        for (auto it = std::ranges::begin(range); it != std::ranges::end(range); ++it)
        {
            using RangeValue = std::ranges::range_value_t<decltype(range)>;
            if constexpr (std::same_as<RangeValue, InnertubeObjects::BackstagePost>)
                addBackstagePostToList(list, *it);
            else if constexpr (std::same_as<RangeValue, InnertubeObjects::Channel>)
                addChannelRendererToList(list, *it);
            else if constexpr (std::same_as<RangeValue, InnertubeObjects::Notification>)
                addNotificationToList(list, *it);
            else if constexpr (std::same_as<RangeValue, InnertubeObjects::Reel> || std::same_as<RangeValue, InnertubeObjects::Video>)
                addVideoRendererToList(list, *it);
            QCoreApplication::processEvents();
        }
    }

    template<typename T>
    static T findParent(QWidget* widget)
    {
        QWidget* parentWidget = widget->parentWidget();
        while (parentWidget)
        {
            if (T castedWidget = qobject_cast<T>(parentWidget))
                return castedWidget;
            parentWidget = parentWidget->parentWidget();
        }

        return nullptr;
    }
private:
    static VideoRenderer* constructVideoRenderer(QListWidget* list);
};
