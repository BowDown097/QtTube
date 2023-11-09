#include "eastereggs.h"
#include "ui/views/viewcontroller.h"
#include <array>
#include <QApplication>
#include <QLineEdit>

namespace EasterEggs
{
    constexpr std::array<int, 5> bk = { 0x42, 0x41, 0x53, 0x45, 0x44 };
    constexpr std::array<int, 5> sk = { 0x53, 0x4e, 0x45, 0x45, 0x44 };
    static int bp = 0;
    static int sp = 0;

    void checkEasterEggs(QKeyEvent* keyEvent)
    {
        QWidget* focused = qApp->focusWidget();
        if (focused && qobject_cast<QLineEdit*>(focused)) // make sure the user isn't typing in a text input
            return;

        int key = keyEvent->key();
        if (key == bk[bp])
            bp++;
        else if (key == sk[sp])
            sp++;
        else
            bp = sp = 0;

        if (bp == bk.size())
        {
            bp = 0;
            ViewController::loadVideo(QByteArray::fromBase64("\x54\x44\x46\x61\x64\x56\x52\x71\x63\x55\x56\x59\x4f\x54\x67"));
        }
        else if (sp == sk.size())
        {
            sp = 0;
            ViewController::loadVideo(QByteArray::fromBase64("\x56\x44\x4e\x46\x62\x45\x6c\x32\x54\x6b\x4a\x5a\x61\x6a\x41"));
        }
    }
}
