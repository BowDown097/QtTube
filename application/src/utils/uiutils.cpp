#include "uiutils.h"
#include "mainwindow.h"
#include "qttubeapplication.h"
#include "ui/widgets/dynamiclistwidgetitem.h"
#include "ui/widgets/labels/tubelabel.h"
#include "ui/widgets/renderers/browsechannelrenderer.h"
#include "ui/widgets/renderers/browsenotificationrenderer.h"
#include "ui/widgets/renderers/video/browsevideorenderer.h"
#include "ui/widgets/renderers/video/gridvideorenderer.h"
#include "ui/widgets/topbar/topbar.h"
#include <QClipboard>
#include <QFile>
#include <QLayout>
#include <QPainter>
#include <QStyleFactory>

constexpr QLatin1String DarkStylesheet(R"(
    QLineEdit {
        background: rgb(42,42,42);
        border: 1px solid rgb(30,30,30);
    }
    QListView::item {
        background: rgb(49,49,49);
    }
    QComboBox, QMessageBox, QPushButton, QScrollBar::vertical, QSpinBox, QTabBar::tab {
        background: rgb(42,42,42);
    }
    QPushButton::hover, QTabBar::tab::hover, QTabBar::tab::selected {
        background: rgb(30,30,30);
    }
    QTabWidget::pane {
        border-color: rgb(30,30,30);
    }
    QToolButton {
        background: transparent;
        border: 1px solid rgb(30,30,30);
    }
)");

namespace UIUtils
{
    QString g_defaultStyle;

    void addChannelToList(QListWidget* list, const QtTubePlugin::Channel& channel, PluginData* plugin)
    {
        BrowseChannelRenderer* renderer = new BrowseChannelRenderer(plugin);
        renderer->setData(channel);
        addWidgetToList(list, renderer);
    }

    void addNotificationToList(QListWidget* list, const QtTubePlugin::Notification& notification)
    {
        BrowseNotificationRenderer* renderer = new BrowseNotificationRenderer;
        renderer->setData(notification);
        addWidgetToList(list, renderer);
    }

    QListWidgetItem* addResizingWidgetToList(QListWidget* list, QWidget* widget)
    {
        DynamicListWidgetItem* item = new DynamicListWidgetItem(list);
        item->setWidget(widget);
        return item;
    }

    void addSeparatorToList(QListWidget* list)
    {
        QFrame* line = new QFrame;
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        QSize hint = line->sizeHint();
        if (list->flow() == QListWidget::LeftToRight)
            hint.setWidth(QWIDGETSIZE_MAX);

        QListWidgetItem* item = new QListWidgetItem;
        item->setSizeHint(hint);
        list->addItem(item);
        list->setItemWidget(item, line);
    }

    void addShelfTitleToList(QListWidget* list, const QString& title)
    {
        if (title.isEmpty())
            return;

        TubeLabel* shelfLabel = new TubeLabel(title);
        shelfLabel->setFont(QFont(shelfLabel->font().toString(), shelfLabel->font().pointSize() + 2));

        QSize hint = shelfLabel->sizeHint();
        if (list->flow() == QListWidget::LeftToRight)
            hint.setWidth(QWIDGETSIZE_MAX);

        QListWidgetItem* item = new QListWidgetItem;
        item->setSizeHint(hint);
        list->addItem(item);
        list->setItemWidget(item, shelfLabel);
    }

    void addVideoToList(QListWidget* list, const QtTubePlugin::Video& video, PluginData* plugin)
    {
        if (qtTubeApp->settings().videoIsFiltered(video))
            return;

        VideoRenderer* renderer = constructVideoRenderer(list, plugin);
        renderer->setData(video);
        addWidgetToList(list, renderer);
    }

    QListWidgetItem* addWidgetToList(QListWidget* list, QWidget* widget)
    {
        QListWidgetItem* item = new QListWidgetItem;
        item->setSizeHint(widget->sizeHint());
        list->addItem(item);
        list->setItemWidget(item, widget);
        return item;
    }

    void clearLayout(QLayout* layout)
    {
        while (QLayoutItem* item = layout->takeAt(0))
        {
            if (QWidget* widget = item->widget())
                widget->deleteLater();
            if (QLayout* childLayout = item->layout())
                clearLayout(childLayout);
            delete item;
        }
    }

    VideoRenderer* constructVideoRenderer(QListWidget* list, PluginData* plugin)
    {
        VideoRenderer* renderer;
        if (list->flow() == QListWidget::LeftToRight)
            renderer = new GridVideoRenderer(plugin, list);
        else
            renderer = new BrowseVideoRenderer(plugin, list);

        return renderer;
    }

    void copyToClipboard(const QString& text)
    {
        QClipboard* clipboard = qApp->clipboard();
        clipboard->setText(text, QClipboard::Clipboard);

        if (clipboard->supportsSelection())
            clipboard->setText(text, QClipboard::Selection);
    }

    MainWindow* getMainWindow(bool throwOnFail)
    {
        const QWidgetList widgets = qApp->topLevelWidgets();

        for (QWidget* window : widgets)
            if (MainWindow* mainWindow = qobject_cast<MainWindow*>(window))
                return mainWindow;

        if (throwOnFail)
            throw std::runtime_error("Failed to find main window. This shouldn't happen!");
        else
            return nullptr;
    }

    QIcon iconThemed(const QString& name, const QPalette& pal)
    {
        return QIcon(resolveThemedIconName(name, pal));
    }

    QPixmap pixmapRounded(const QPixmap& pixmap, double xRadius, double yRadius)
    {
        QPixmap rounded(pixmap.size());
        rounded.fill(Qt::transparent);

        QPainter painter(&rounded);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(QBrush(pixmap));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(pixmap.rect(), xRadius, yRadius);
        painter.end();

        return rounded;
    }

    QPixmap pixmapThemed(const QString& name, const QPalette& pal)
    {
        return QPixmap(resolveThemedIconName(name, pal));
    }

    bool preferDark(const QPalette& pal)
    {
        return pal == QPalette()
                ? qApp->palette().alternateBase().color().lightness() < 60
                : pal.alternateBase().color().lightness() < 60;
    }

    QString relativeTimeString(const QDateTime& target, const QDateTime& reference)
    {
        if (target == reference)
            return QLatin1String("now");

        enum DateTimeUnit { YEAR, MONTH, WEEK, DAY, HOUR, MINUTE, SECOND };
        struct DateTimeUnitInfo { int unitDuration; DateTimeUnit unit; int threshold = 1; };
        struct PluralSelector { QLatin1String one; QLatin1String other; };
        struct RDTFSymbol { PluralSelector P; PluralSelector F; };

        static constexpr std::array dateTimeUnits = {
            DateTimeUnitInfo { .unitDuration = int(604800), .unit = WEEK, .threshold = 2 },
            DateTimeUnitInfo { .unitDuration = int(86400), .unit = DAY },
            DateTimeUnitInfo { .unitDuration = int(3600), .unit = HOUR },
            DateTimeUnitInfo { .unitDuration = int(60), .unit = MINUTE },
            DateTimeUnitInfo { .unitDuration = int(1), .unit = SECOND }
        };

        static constexpr std::array rdtfSymbols = {
            RDTFSymbol { // year
                .P = {
                    .one = QLatin1String("%1 year ago"),
                    .other = QLatin1String("%1 years ago")
                },
                .F = {
                    .one = QLatin1String("in %1 year"),
                    .other = QLatin1String("in %1 years")
                }
            },
            RDTFSymbol { // month
                .P = {
                    .one = QLatin1String("%1 month ago"),
                    .other = QLatin1String("%1 months ago")
                },
                .F = {
                    .one = QLatin1String("in %1 month"),
                    .other = QLatin1String("in %1 months")
                }
            },
            RDTFSymbol { // week
                .P = {
                    .one = QLatin1String("%1 week ago"),
                    .other = QLatin1String("%1 weeks ago")
                },
                .F = {
                    .one = QLatin1String("in %1 week"),
                    .other = QLatin1String("in %1 weeks")
                }
            },
            RDTFSymbol { // day
                .P = {
                    .one = QLatin1String("%1 day ago"),
                    .other = QLatin1String("%1 days ago")
                },
                .F = {
                    .one = QLatin1String("in %1 day"),
                    .other = QLatin1String("in %1 days")
                }
            },
            RDTFSymbol { // hour
                .P = {
                    .one = QLatin1String("%1 hour ago"),
                    .other = QLatin1String("%1 hours ago")
                },
                .F = {
                    .one = QLatin1String("in %1 hour"),
                    .other = QLatin1String("in %1 hours")
                }
            },
            RDTFSymbol { // minute
                .P = {
                    .one = QLatin1String("%1 minute ago"),
                    .other = QLatin1String("%1 minutes ago")
                },
                .F = {
                    .one = QLatin1String("in %1 minute"),
                    .other = QLatin1String("in %1 minutes")
                }
            },
            RDTFSymbol { // second
                .P = {
                    .one = QLatin1String("%1 second ago"),
                    .other = QLatin1String("%1 seconds ago")
                },
                .F = {
                    .one = QLatin1String("in %1 second"),
                    .other = QLatin1String("in %1 seconds")
                }
            },
        };

        auto format = [](qint64 num, DateTimeUnit unit) -> QString {
            bool future = num > 0;
            const PluralSelector& sel = future ? rdtfSymbols[unit].F : rdtfSymbols[unit].P;

            num = std::abs(num);
            QLatin1String pattern = num == 1 ? sel.one : sel.other;
            return pattern.arg(QString::number(num));
        };

        bool future = reference > target;
        const QDateTime& start = future ? target : reference;
        const QDateTime& end = future ? reference : target;

        int v = 0;
        while (start.addMonths(12 * (v + 1)) < end) ++v;
        if (v > 0)
            return format(future ? v : -v, YEAR);

        v = 0;
        while (start.addMonths(v + 1) < end) ++v;
        if (v > 0)
            return format(future ? v : -v, MONTH);

        qint64 secs = start.secsTo(end);
        for (const DateTimeUnitInfo& u : dateTimeUnits)
            if (qint64 n = secs / u.unitDuration; n >= u.threshold)
                return format(future ? n : -n, u.unit);

        return {};
    }

    QString resolveThemedIconName(const QString& name, const QPalette& pal)
    {
        const QString baseFile = ":/" + name + ".svg";
        const QString lightFile = ":/" + name + "-light.svg";
        return QFile::exists(lightFile) && preferDark(pal) ? lightFile : baseFile;
    }

    void setAppStyle(const QString& styleName, bool dark)
    {
        if (styleName == "Default" && qApp->style()->objectName() != g_defaultStyle)
            qApp->setStyle(QStyleFactory::create(g_defaultStyle));
        else if (QStyle* style = QStyleFactory::create(styleName))
            qApp->setStyle(style);

        MainWindow* mainWindow = getMainWindow(false);
        if (dark)
        {
            QPalette darkPalette;
            darkPalette.setColor(QPalette::Window, QColor(49,49,49));
            darkPalette.setColor(QPalette::WindowText, Qt::white);
            darkPalette.setColor(QPalette::Base, QColor(34,34,34));
            darkPalette.setColor(QPalette::AlternateBase, QColor(42,42,42));
            darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
            darkPalette.setColor(QPalette::ToolTipText, Qt::white);
            darkPalette.setColor(QPalette::Text, Qt::white);
            darkPalette.setColor(QPalette::Button, QColor(48,48,48));
            darkPalette.setColor(QPalette::ButtonText, Qt::white);
            darkPalette.setColor(QPalette::BrightText, Qt::red);
            darkPalette.setColor(QPalette::Link, QColor(47,163,198));
            darkPalette.setColor(QPalette::Highlight, QColor(145,205,92));
            darkPalette.setColor(QPalette::HighlightedText, Qt::black);
            darkPalette.setColor(QPalette::PlaceholderText, Qt::darkGray);
            qApp->setPalette(darkPalette);
            qApp->setStyleSheet(DarkStylesheet);

            if (mainWindow)
                mainWindow->topbar()->updatePalette(darkPalette);
        }
        else if (qApp->styleSheet() == DarkStylesheet)
        {
            qApp->setPalette(qApp->style()->standardPalette());
            qApp->setStyleSheet(QString());

            if (mainWindow)
                mainWindow->topbar()->updatePalette(qApp->palette());
        }
    }

    void setTabsEnabled(QTabWidget* widget, bool enabled, std::initializer_list<int> indexes)
    {
        for (int i : indexes)
            widget->setTabEnabled(i, enabled);
    }
}
