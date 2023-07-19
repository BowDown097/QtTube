#include "searchbox.h"
#include "ui/uiutilities.h"

SearchBox::SearchBox(QWidget* parent)
    : QWidget(parent),
      layout(new QHBoxLayout(this)),
      searchButton(new ExtToolButton(this)),
      searchForm(new QLineEdit(this)),
      searchTypeActionLink(new QAction(UIUtilities::iconThemed("link"), "Link/ID", this)),
      searchTypeActionQuery(new QAction(UIUtilities::iconThemed("search"), "Query", this)),
      searchTypeMenu(new ExtMenu(this))
{
    setFixedHeight(35);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    connect(searchTypeActionLink, &QAction::triggered, this, [this] { emit searchRequested(searchForm->text(), SearchType::ByLink); });
    connect(searchTypeActionQuery, &QAction::triggered, this, [this] { emit searchRequested(searchForm->text(), SearchType::ByQuery); });

    searchTypeMenu->addAction(searchTypeActionQuery);
    searchTypeMenu->addAction(searchTypeActionLink);
    connect(searchTypeMenu, &ExtMenu::switchActionRequested, searchButton, &QToolButton::setDefaultAction);

    searchButton->setDefaultAction(searchTypeActionQuery);
    searchButton->setFixedSize(35, 35);
    searchButton->setIconSize(QSize(20, 20));
    searchButton->setMenu(searchTypeMenu);
    searchButton->setPopupMode(QToolButton::DelayedPopup);
    searchButton->setToolButtonStyle(Qt::ToolButtonIconOnly);

    searchForm->setClearButtonEnabled(true);
    searchForm->setFixedHeight(35);
    searchForm->setPlaceholderText("Search");
    connect(searchForm, &QLineEdit::returnPressed, this, [this] {
        emit searchRequested(
            searchForm->text(),
            searchButton->defaultAction() == searchTypeActionQuery ? SearchType::ByQuery : SearchType::ByLink
        );
    });

    layout->addWidget(searchForm);
    layout->addWidget(searchButton);
}

void SearchBox::updatePalette(const QPalette& pal)
{
    setPalette(pal);
    searchTypeActionLink->setIcon(UIUtilities::iconThemed("link", pal));
    searchTypeActionQuery->setIcon(UIUtilities::iconThemed("search", pal));
}
