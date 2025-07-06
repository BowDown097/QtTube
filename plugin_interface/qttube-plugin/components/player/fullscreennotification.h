#pragma once
#include <QWidget>

namespace QtTubePlugin
{
    class FullScreenNotification : public QWidget
    {
        Q_OBJECT
    public:
        explicit FullScreenNotification(QWidget* parent = nullptr);
        QSize sizeHint() const override;
        QString text() const;
    protected:
        void paintEvent(QPaintEvent* event) override;
        void showEvent(QShowEvent* event) override;
    private:
        bool m_previouslyVisible{};
    signals:
        void shown();
    };
}
