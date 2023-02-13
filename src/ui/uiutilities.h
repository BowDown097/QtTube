#ifndef UIUTILITIES_H
#define UIUTILITIES_H
#include <initializer_list>
#include <QLabel>
#include <QLayout>
#include <QTabWidget>

class UIUtilities
{
public:
    static void clearLayout(QLayout* layout);
    static void copyToClipboard(const QString& text);
    static void elide(QLabel* label, int targetWidth);
    static void setMaximumLines(QWidget* widget, int lines);
    static void setTabsEnabled(QTabWidget* widget, bool enabled, std::initializer_list<int> indexes);

    template<typename T>
    static T findParent(QWidget* widget);
};

#include "uiutilities.tpp"

#endif // UIUTILITIES_H
