#pragma once
#include "ui/widgets/labels/tubelabel.hpp"

class QProgressBar;

class VideoThumbnailWidget : public TubeLabel
{
    Q_OBJECT
public:
    explicit VideoThumbnailWidget(QWidget* parent = nullptr);
    void setLengthText(const QString& text) { m_lengthLabel->setText(text); }
    void setProgress(int progress, int length);
protected:
    void resizeEvent(QResizeEvent* event) override;
private:
    QLabel* m_lengthLabel;
    QProgressBar* m_progressBar;
};
