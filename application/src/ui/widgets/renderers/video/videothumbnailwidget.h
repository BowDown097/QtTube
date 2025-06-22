#pragma once
#include "ui/widgets/labels/tubelabel.h"

class QProgressBar;

class VideoThumbnailWidget : public TubeLabel
{
    Q_OBJECT
public:
    explicit VideoThumbnailWidget(QWidget* parent = nullptr);
    void setLengthText(const QString& text) { m_lengthLabel->setText(text); }
    void setProgress(int progress, int length);
    void setSourceIconUrl(const char* sourceIconUrl);
protected:
    void resizeEvent(QResizeEvent* event) override;
private:
    bool m_hasSourceIcon{};
    QLabel* m_lengthLabel;
    QProgressBar* m_progressBar;
    TubeLabel* m_sourceIconLabel;
};
