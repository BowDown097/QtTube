#pragma once
#include "ui/widgets/continuablelistwidget.hpp"
#include <QTabWidget>

class ReloadButton;

#define CONTINUE_CONNECT(widget, function, ...) \
    connect(widget, &ContinuableListWidget::continuationReady, this, [=, this] { \
        if (widget->continuationData.has_value()) \
            BrowseHelper::instance()->function(m_plugin, widget __VA_OPT__(,) __VA_ARGS__); \
    })

class BrowserTabWidget : public QTabWidget
{
public:
    explicit BrowserTabWidget(QWidget* parent = nullptr);
    virtual void browse() = 0;
    QAction* makeReloadAction();
    virtual void reloadCurrentTab(ContinuableListWidget* list) = 0;
    void repolish();
protected:
    template<typename... Indices>
    void setTabsVisible(bool visible, Indices... indices)
    {
        (setTabVisible(indices, visible), ...);
    }
private:
    ReloadButton* m_reloadButton;
    ContinuableListWidget* m_reloadingList;
private slots:
    void reloadFrameChanged();
    void tryReload();
};
