#ifndef RICHITEMDELEGATE_H
#define RICHITEMDELEGATE_H
#include <QStyledItemDelegate>

class RichItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit RichItemDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}
    QString anchorAt(const QString& html, const QPoint& point) const;
protected:
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

#endif // RICHITEMDELEGATE_H
