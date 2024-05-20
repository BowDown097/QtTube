#pragma once
#include <QWidget>

class ExtMenu;
class ExtToolButton;
class QHBoxLayout;
class QLineEdit;

class SearchBox : public QWidget
{
    Q_OBJECT
public:
    enum class SearchType { ByQuery, ByLink };
    explicit SearchBox(QWidget* parent = nullptr);
    void updatePalette(const QPalette& pal);
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
