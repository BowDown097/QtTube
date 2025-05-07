#pragma once
#include <QWebEngineView>

class FullScreenNotification;

class FullScreenWindow : public QWidget
{
    Q_OBJECT
public:
    explicit FullScreenWindow(QWebEngineView* oldView, QWidget* parent = nullptr);
    ~FullScreenWindow();
protected:
    void resizeEvent(QResizeEvent* event) override;
private:
    FullScreenNotification* m_notification;
    QRect m_oldGeometry;
    QWebEngineView* m_oldView;
    QWebEngineView* m_view;
};
