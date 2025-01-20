#pragma once
#include <QListWidgetItem>

class ResizeEventFilter : public QObject
{
    Q_OBJECT
public:
    explicit ResizeEventFilter(QObject* parent = nullptr) : QObject(parent) {}
protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
signals:
    void resizing();
};

struct DynamicListWidgetItem : QListWidgetItem
{
    explicit DynamicListWidgetItem(QListWidget* listview, int type = Type)
        : QListWidgetItem(listview, type) {}
    explicit DynamicListWidgetItem(const QString& text, QListWidget* listview, int type = Type)
        : QListWidgetItem(text, listview, type) {}
    explicit DynamicListWidgetItem(const QIcon& icon, const QString& text, QListWidget* listview, int type = Type)
        : QListWidgetItem(icon, text, listview, type) {}

    void setWidget(QWidget* widget);
};
