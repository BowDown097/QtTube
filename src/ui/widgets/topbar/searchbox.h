#ifndef SEARCHBOX_H
#define SEARCHBOX_H
#include "extmenu.h"
#include "exttoolbutton.h"
#include <QHBoxLayout>
#include <QLineEdit>

class SearchBox : public QWidget
{
    Q_OBJECT
public:
    enum class SearchType { ByQuery, ByLink };
    explicit SearchBox(QWidget* parent = nullptr);
signals:
    void searchRequested(const QString& query, SearchBox::SearchType searchType);
private:
    QHBoxLayout* layout;
    ExtToolButton* searchButton;
    QLineEdit* searchForm;
    QAction* searchTypeActionLink;
    QAction* searchTypeActionQuery;
    ExtMenu* searchTypeMenu;
};

#endif // SEARCHBOX_H
