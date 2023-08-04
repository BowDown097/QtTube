#ifndef UIUTILS_TPP
#define UIUTILS_TPP

template<typename T>
T UIUtils::findParent(QWidget* widget)
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

#endif // UIUTILS_TPP
