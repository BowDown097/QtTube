#pragma once
#include <QWidget>

namespace QtTubePlugin
{
    class Player : public QWidget
    {
        Q_OBJECT
    public:
        enum PlayerState
        {
            State_Unstarted = -1,
            State_Ended = 0,
            State_Playing = 1,
            State_Paused = 2,
            State_Buffering = 3,
            State_Cued = 5
        };

        using QWidget::QWidget;
    public slots:
        virtual void play(const QString& videoId, int progress) = 0;
        virtual void seek(int progress) = 0;
    signals:
        void copyToClipboardRequested(const QString& text);
        void newState(QtTubePlugin::Player::PlayerState state);
        void progressChanged(qint64 progress, qint64 previousProgress);
        void switchVideoRequested(const QString& videoId);
    };
}
