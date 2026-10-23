#include "browsertabwidget.hpp"
#include "reloadbutton.hpp"
#include "utils/uiutils.hpp"

BrowserTabWidget::BrowserTabWidget(QWidget* parent)
    : QTabWidget(parent),
      m_reloadButton(new ReloadButton(this))
{
    m_reloadButton->setContentsMargins(4, 4, 4, 4);
    m_reloadButton->setFixedWidth(32);
    m_reloadButton->setToolTip("Reload");
    setCornerWidget(m_reloadButton, Qt::TopLeftCorner);

    connect(this, &QTabWidget::currentChanged, this, &BrowserTabWidget::browse);
    connect(m_reloadButton, &QPushButton::clicked, this, &BrowserTabWidget::tryReload);
    connect(m_reloadButton->movie(), &QMovie::frameChanged, this, &BrowserTabWidget::reloadFrameChanged);
}

QAction* BrowserTabWidget::makeReloadAction()
{
    QAction* result = new QAction(this);
    result->setAutoRepeat(false);
    result->setShortcuts(QList<QKeySequence>() << Qt::Key_F5 << QKeySequence(Qt::ControlModifier | Qt::Key_R));
    connect(result, &QAction::triggered, this, &BrowserTabWidget::tryReload);
    return result;
}

void BrowserTabWidget::reloadFrameChanged()
{
    if (!m_reloadingList) [[unlikely]]
        return;

    QMovie* movie = m_reloadButton->movie();
    if (!m_reloadingList->isPopulating() && movie->currentFrameNumber() == movie->frameCount() - 1)
        movie->stop();
}

void BrowserTabWidget::repolish()
{
    UIUtils::repolish(this);
    std::ranges::for_each(findChildren<QWidget*>(), UIUtils::repolish);
    m_reloadButton->movie()->setFileName(UIUtils::resolveThemedIconName("spinner", "gif"));
    m_reloadButton->movie()->jumpToFrame(0);
}

void BrowserTabWidget::tryReload()
{
    auto* list = qobject_cast<ContinuableListWidget*>(currentWidget());
    if (!list || list->isPopulating())
        return;

    m_reloadingList = list;
    m_reloadButton->movie()->start();
    reloadCurrentTab(m_reloadingList);
}