#include "uiutilities.h"
#include <QApplication>

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

// this will be used for the description and perhaps elsewhere
void UIUtilities::setMaximumLines(QWidget* widget, int lines)
{
    QFontMetrics fm(QApplication::font());
    widget->setFixedHeight(fm.lineSpacing() * lines);
}

void UIUtilities::setTabsEnabled(QTabWidget* widget, bool enabled, std::initializer_list<int> indexes)
{
    for (int i : indexes)
        widget->setTabEnabled(i, enabled);
}
