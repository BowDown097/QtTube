#ifndef VIDEOTHUMBNAILWIDGET_H
#define VIDEOTHUMBNAILWIDGET_H
#include "httpreply.h"
#include <QLabel>
#include <QProgressBar>

class VideoThumbnailWidget : public QLabel
{
    Q_OBJECT
public:
    explicit VideoThumbnailWidget(QWidget* parent = nullptr);
    void setLengthText(const QString& text) { lengthLabel->setText(text); }
    void setPreferredSize(const QSize& size);
    void setProgress(int progress, int length);
    void setUrl(const QString& url);
protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent*) override;
#else
    void enterEvent(QEvent*) override;
#endif
    void leaveEvent(QEvent*) override;
    void mousePressEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
private:
    QLabel* lengthLabel;
    QSize preferredSize;
    QProgressBar* progressBar;
private slots:
    void setData(const HttpReply& reply);
signals:
    void clicked();
    void thumbnailSet();
};

#endif // VIDEOTHUMBNAILWIDGET_H
