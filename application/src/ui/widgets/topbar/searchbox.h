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
protected:
    void changeEvent(QEvent* event) override;
private:
    QHBoxLayout* m_layout;
    QLineEdit* m_lineEdit;
    ExtToolButton* m_searchButton;
    QAction* m_searchTypeActionLink;
    QAction* m_searchTypeActionQuery;
    ExtMenu* m_searchTypeMenu;
signals:
    void searchRequested(const QString& query, SearchBox::SearchType searchType);
};
