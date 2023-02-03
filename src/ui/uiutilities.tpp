#ifndef UIUTILITIES_TPP
#define UIUTILITIES_TPP

template<typename T>
T UIUtilities::findParent(QWidget* widget)
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

#endif // UIUTILITIES_TPP
