#pragma once
#include "ui/widgets/clickablewidget.h"
#include <QLabel>

class HttpReply;
class QProgressBar;

class VideoThumbnailWidget : public ClickableWidget<QLabel>
{
    Q_OBJECT
public:
    explicit VideoThumbnailWidget(QWidget* parent = nullptr);
    void setLengthText(const QString& text) { m_lengthLabel->setText(text); }
    void setProgress(int progress, int length);
    void setSourceIconUrl(const char* sourceIconUrl);
    void setUrl(const QString& url);
protected:
    void resizeEvent(QResizeEvent* event) override;
private:
    bool m_hasSourceIcon{};
    QLabel* m_lengthLabel;
    QProgressBar* m_progressBar;
    QLabel* m_sourceIconLabel;
private slots:
    void setData(const HttpReply& reply);
    void setSourceIconData(const HttpReply& reply);
};
