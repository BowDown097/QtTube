#include "richitemdelegate.h"
#include <QAbstractTextDocumentLayout>
#include <QPainter>
#include <QTextDocument>

QString RichItemDelegate::anchorAt(const QString& html, const QPoint& point) const
{
    QTextDocument doc;
    doc.setHtml(html);

    QAbstractTextDocumentLayout* layout = doc.documentLayout();
    return layout ? layout->anchorAt(point) : QString();
}

void RichItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem optionCpy = option;

    initStyleOption(&optionCpy, index);
    painter->save();

    QTextDocument doc;
    doc.setHtml(optionCpy.text);

    optionCpy.text = "";
    optionCpy.widget->style()->drawControl(QStyle::CE_ItemViewItem, &optionCpy, painter);

    painter->translate(optionCpy.rect.left(), optionCpy.rect.top());

    QRect clip(0, 0, optionCpy.rect.width(), optionCpy.rect.height());
    doc.drawContents(painter, clip);

    painter->restore();
}

QSize RichItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    initStyleOption(const_cast<QStyleOptionViewItem*>(&option), index);

    QTextDocument doc;
    doc.setHtml(option.text);
    doc.setTextWidth(option.rect.width());

    return QSize(doc.idealWidth(), doc.size().height());
}
