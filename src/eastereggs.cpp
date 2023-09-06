#include "eastereggs.h"
#include "ui/views/viewcontroller.h"
#include <array>
#include <QApplication>
#include <QLineEdit>

namespace EasterEggs
{
    constexpr std::array<Qt::Key, 5> basedKeys = { Qt::Key_B, Qt::Key_A, Qt::Key_S, Qt::Key_E, Qt::Key_D };
    constexpr std::array<Qt::Key, 5> sneedKeys = { Qt::Key_S, Qt::Key_N, Qt::Key_E, Qt::Key_E, Qt::Key_D };
    static int basedProgress = 0;
    static int sneedProgress = 0;

    void checkEasterEggs(QKeyEvent* keyEvent)
    {
        QWidget* focused = qApp->focusWidget();
        if (focused && qobject_cast<QLineEdit*>(focused)) // make sure the user isn't typing in a text input
            return;

        Qt::Key key = static_cast<Qt::Key>(keyEvent->key());
        if (key == basedKeys[basedProgress])
            basedProgress++;
        else if (key == sneedKeys[sneedProgress])
            sneedProgress++;
        else
            basedProgress = sneedProgress = 0;

        if (basedProgress == basedKeys.size())
        {
            basedProgress = 0;
            ViewController::loadVideo("L1ZuTjqEX98");
        }
        else if (sneedProgress == sneedKeys.size())
        {
            sneedProgress = 0;
            ViewController::loadVideo("T3ElIvNBYj0");
        }
    }
}
