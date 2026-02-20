#include "searchbox.h"
#include "extmenu.h"
#include "exttoolbutton.h"
#include "utils/uiutils.h"
#include <QBoxLayout>
#include <QEvent>
#include <QLineEdit>

SearchBox::SearchBox(QWidget* parent)
    : QWidget(parent),
      m_layout(new QHBoxLayout(this)),
      m_lineEdit(new QLineEdit(this)),
      m_searchButton(new ExtToolButton(this)),
      m_searchTypeActionLink(new QAction(UIUtils::iconThemed("link"), "Link/ID", this)),
      m_searchTypeActionQuery(new QAction(UIUtils::iconThemed("search"), "Query", this)),
      m_searchTypeMenu(new ExtMenu(this))
{
    setFixedHeight(35);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    connect(m_searchTypeActionLink, &QAction::triggered, this, [this] { emit searchRequested(m_lineEdit->text(), SearchType::ByLink); });
    connect(m_searchTypeActionQuery, &QAction::triggered, this, [this] { emit searchRequested(m_lineEdit->text(), SearchType::ByQuery); });

    m_searchTypeMenu->addAction(m_searchTypeActionQuery);
    m_searchTypeMenu->addAction(m_searchTypeActionLink);
    connect(m_searchTypeMenu, &ExtMenu::switchActionRequested, m_searchButton, &QToolButton::setDefaultAction);

    m_searchButton->setDefaultAction(m_searchTypeActionQuery);
    m_searchButton->setFixedSize(35, 35);
    m_searchButton->setIconSize(QSize(20, 20));
    m_searchButton->setMenu(m_searchTypeMenu);
    m_searchButton->setPopupMode(QToolButton::DelayedPopup);
    m_searchButton->setToolButtonStyle(Qt::ToolButtonIconOnly);

    m_lineEdit->setClearButtonEnabled(true);
    m_lineEdit->setFixedHeight(35);
    m_lineEdit->setPlaceholderText("Search");
    connect(m_lineEdit, &QLineEdit::returnPressed, this, [this] {
        emit searchRequested(
            m_lineEdit->text(),
            m_searchButton->defaultAction() == m_searchTypeActionQuery ? SearchType::ByQuery : SearchType::ByLink
        );
    });

    m_layout->addWidget(m_lineEdit);
    m_layout->addWidget(m_searchButton);
}

void SearchBox::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::PaletteChange)
    {
        m_searchTypeActionLink->setIcon(UIUtils::iconThemed("link"));
        m_searchTypeActionQuery->setIcon(UIUtils::iconThemed("search"));
    }
}
