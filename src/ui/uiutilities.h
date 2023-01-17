#ifndef UIUTILITIES_H
#define UIUTILITIES_H
#include <initializer_list>
#include <QLayout>
#include <QTabWidget>

class UIUtilities
{
public:
    static void clearLayout(QLayout* layout);
    static void copyToClipboard(const QString& text);
    static void setMaximumLines(QWidget* widget, int lines);
    static void setTabsEnabled(QTabWidget* widget, bool enabled, std::initializer_list<int> indexes);
};

#endif // UIUTILITIES_H
