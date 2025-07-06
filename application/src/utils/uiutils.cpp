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

    void addChannelToList(QListWidget* list, const QtTube::PluginChannel& channel)
    {
        BrowseChannelRenderer* renderer = new BrowseChannelRenderer;
        renderer->setData(channel);
        addWidgetToList(list, renderer);
    }

    void addNotificationToList(QListWidget* list, const QtTube::PluginNotification& notification)
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

    void addVideoToList(QListWidget* list, const QtTube::PluginVideo& video)
    {
        if (qtTubeApp->settings().videoIsFiltered(video))
            return;

        VideoRenderer* renderer = constructVideoRenderer(list);
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

    VideoRenderer* constructVideoRenderer(QListWidget* list)
    {
        VideoRenderer* renderer;
        if (list->flow() == QListWidget::LeftToRight)
            renderer = new GridVideoRenderer(list);
        else
            renderer = new BrowseVideoRenderer(list);

        return renderer;
    }

    void copyToClipboard(const QString& text)
    {
        QClipboard* clipboard = qApp->clipboard();
        clipboard->setText(text, QClipboard::Clipboard);

        if (clipboard->supportsSelection())
            clipboard->setText(text, QClipboard::Selection);
    }

    QMainWindow* getMainWindow()
    {
        const QWidgetList widgets = qApp->topLevelWidgets();
        for (QWidget* window : widgets)
            if (QMainWindow* mainWindow = qobject_cast<QMainWindow*>(window))
                return mainWindow;
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
            MainWindow::topbar()->updatePalette(darkPalette);
        }
        else if (qApp->styleSheet() == DarkStylesheet)
        {
            qApp->setPalette(qApp->style()->standardPalette());
            qApp->setStyleSheet(QString());
            MainWindow::topbar()->updatePalette(qApp->palette());
        }

    #ifdef Q_OS_WIN // for some reason, wrong palette is applied to topbar on windows
        MainWindow::topbar()->updatePalette(qApp->palette());
    #endif
    }

    void setTabsEnabled(QTabWidget* widget, bool enabled, std::initializer_list<int> indexes)
    {
        for (int i : indexes)
            widget->setTabEnabled(i, enabled);
    }
}
