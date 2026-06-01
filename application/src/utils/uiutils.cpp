#include "uiutils.h"
#include "mainwindow.h"
#include "qttubeapplication.h"
#include "ui/widgets/dynamiclistwidgetitem.h"
#include "ui/widgets/labels/tubelabel.h"
#include "ui/widgets/renderers/browsechannelrenderer.h"
#include "ui/widgets/renderers/browsenotificationrenderer.h"
#include "ui/widgets/renderers/video/browsevideorenderer.h"
#include "ui/widgets/renderers/video/gridvideorenderer.h"
#include <QClipboard>
#include <QFile>
#include <QLayout>
#include <QPainter>
#include <QStyleFactory>
#include <unicode/reldatefmt.h>

namespace
{
    const QString darkStylesheet = QStringLiteral(R"(
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
}

namespace UIUtils
{
    QString g_defaultStyle;

    void addChannelToList(QListWidget* list, const QtTubePlugin::Channel& channel, PluginEntry* plugin)
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

    void addVideoToList(QListWidget* list, const QtTubePlugin::Video& video, PluginEntry* plugin)
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

    VideoRenderer* constructVideoRenderer(QListWidget* list, PluginEntry* plugin)
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

    QIcon iconThemed(const QString& name)
    {
        return QIcon(resolveThemedIconName(name));
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

    QPixmap pixmapThemed(const QString& name)
    {
        return QPixmap(resolveThemedIconName(name));
    }

    bool preferDark()
    {
        return qApp->palette().color(QPalette::Window).lightness() < 110;
    }

    QString relativeTimeString(qint64 msecs)
    {
        constexpr qint64 cutoffs[] = {
            60,
            3600,
            86400,
            86400 * 7,
            86400 * 30,
            86400 * 365,
            std::numeric_limits<qint64>::max()
        };
        constexpr URelativeDateTimeUnit units[] = {
            UDAT_REL_UNIT_SECOND,
            UDAT_REL_UNIT_MINUTE,
            UDAT_REL_UNIT_HOUR,
            UDAT_REL_UNIT_DAY,
            UDAT_REL_UNIT_WEEK,
            UDAT_REL_UNIT_MONTH,
            UDAT_REL_UNIT_YEAR
        };

        qint64 deltaSecs = std::llround((msecs - QDateTime::currentMSecsSinceEpoch()) / 1000.0);
        auto unitIt = std::ranges::find_if(cutoffs, [=](qint64 c) { return c > std::abs(deltaSecs); });
        size_t unitIndex = std::distance(std::begin(cutoffs), unitIt);
        qint64 divisor = unitIndex ? cutoffs[unitIndex - 1] : 1;

        UErrorCode status = U_ZERO_ERROR;
        static icu::RelativeDateTimeFormatter rtf(icu::Locale::getDefault(), status);
        if (U_FAILURE(status))
            throw std::runtime_error("Failed to make RelativeDateTimeFormatter");

        icu::FormattedRelativeDateTime formatted = rtf.formatNumericToValue(
            deltaSecs / divisor, units[unitIndex], status);
        if (U_FAILURE(status))
            throw std::runtime_error("formatNumericToValue failed");

        icu::UnicodeString str = formatted.toTempString(status);
        if (U_FAILURE(status))
            throw std::runtime_error("toTempString failed");

        return QString(reinterpret_cast<const QChar*>(str.getBuffer()), str.length());
    }

    void repolish(QWidget* widget)
    {
        widget->style()->polish(widget);
    }

    QString resolveThemedIconName(const QString& name)
    {
        const QString baseFile = ":/" % name % ".svg";
        const QString lightFile = ":/" % name % "-light.svg";
        return QFile::exists(lightFile) && preferDark() ? lightFile : baseFile;
    }

    void setAppStyle(const QString& styleName, bool dark)
    {
        if (styleName == "Default" && qApp->style()->objectName() != g_defaultStyle)
            qApp->setStyle(QStyleFactory::create(g_defaultStyle));
        else if (QStyle* style = QStyleFactory::create(styleName))
            qApp->setStyle(style);

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
            qApp->setStyleSheet(darkStylesheet);
        }
        else if (qApp->styleSheet() == darkStylesheet)
        {
            qApp->setPalette(qApp->style()->standardPalette());
            qApp->setStyleSheet(QString());
        }
    }

    void setTabsEnabled(QTabWidget* widget, bool enabled, std::initializer_list<int> indexes)
    {
        for (int i : indexes)
            widget->setTabEnabled(i, enabled);
    }
}
