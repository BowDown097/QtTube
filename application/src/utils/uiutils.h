#pragma once
#include <initializer_list>
#include <QWidget>

namespace QtTubePlugin { struct Channel; struct Notification; struct Video; }

class QLayout;
class QListWidget;
class QListWidgetItem;
class QMainWindow;
class QTabWidget;
class VideoRenderer;

namespace UIUtils
{
    extern QString g_defaultStyle;

    void addChannelToList(QListWidget* list, const QtTubePlugin::Channel& channel);
    void addNotificationToList(QListWidget* list, const QtTubePlugin::Notification& notification);
    QListWidgetItem* addResizingWidgetToList(QListWidget* list, QWidget* widget);
    void addSeparatorToList(QListWidget* list);
    void addShelfTitleToList(QListWidget* list, const QString& title);
    void addVideoToList(QListWidget* list, const QtTubePlugin::Video& video);
    QListWidgetItem* addWidgetToList(QListWidget* list, QWidget* widget);
    void clearLayout(QLayout* layout);
    VideoRenderer* constructVideoRenderer(QListWidget* list);
    void copyToClipboard(const QString& text);
    QMainWindow* getMainWindow();
    QIcon iconThemed(const QString& name, const QPalette& pal = {});
    QPixmap pixmapRounded(const QPixmap& pixmap, double xRadius = 9999, double yRadius = 9999);
    QPixmap pixmapThemed(const QString& name, const QPalette& pal = {});
    bool preferDark(const QPalette& pal = {});
    QString resolveThemedIconName(const QString& name, const QPalette& pal = {});
    void setAppStyle(const QString& styleName, bool dark);
    void setTabsEnabled(QTabWidget* widget, bool enabled, std::initializer_list<int> indexes);

    template<typename T>
    T findParent(QWidget* widget)
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
};
