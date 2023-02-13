#include "uiutilities.h"
#include <QApplication>
#include <QClipboard>
#include <QThread>

void UIUtilities::clearLayout(QLayout* layout)
{
    while (QLayoutItem* item = layout->takeAt(0))
    {
        if (QWidget* widget = item->widget())
            widget->deleteLater();
        if (QLayout* childLayout = item->layout())
            clearLayout(childLayout);
        delete item;
    }
}

void UIUtilities::copyToClipboard(const QString& text)
{
    QClipboard* clipboard = qApp->clipboard();
    clipboard->setText(text, QClipboard::Clipboard);

    if (clipboard->supportsSelection())
        clipboard->setText(text, QClipboard::Selection);

// apparently a workaround for copying to clipboard sometimes not working on linux. see https://www.medo64.com/2019/12/copy-to-clipboard-in-qt/
#ifdef Q_OS_LINUX
    QThread::msleep(1);
#endif
}

void UIUtilities::elide(QLabel* label, int targetWidth)
{
    QFontMetrics fm(label->font());
    QString elidedText = fm.elidedText(label->text(), Qt::ElideRight, targetWidth);
    label->setFixedWidth(targetWidth);
    label->setText(elidedText);
}

// this will be used for the description and perhaps elsewhere
void UIUtilities::setMaximumLines(QWidget* widget, int lines)
{
    QFontMetrics fm(qApp->font());
    widget->setFixedHeight(fm.lineSpacing() * lines);
}

void UIUtilities::setTabsEnabled(QTabWidget* widget, bool enabled, std::initializer_list<int> indexes)
{
    for (int i : indexes)
        widget->setTabEnabled(i, enabled);
}
