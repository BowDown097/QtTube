#ifndef UIUTILITIES_TPP
#define UIUTILITIES_TPP

namespace UIUtilities
{
    template<typename T>
    T findParent(QWidget* widget)
    {
        QWidget* parentWidget = widget->parentWidget();
        while (parentWidget)
        {
            if (T castedWidget = qobject_cast<T>(parentWidget))
                return castedWidget;
            parentWidget = parentWidget->parentWidget();
        }

        return nullptr;
    }
}

#endif // UIUTILITIES_TPP
