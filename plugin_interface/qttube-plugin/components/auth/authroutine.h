#pragma once
#include <QObject>

namespace QtTubePlugin
{
    class AuthRoutine : public QObject
    {
        Q_OBJECT
    public:
        virtual void start() = 0;
    signals:
        void success();
    };
}
