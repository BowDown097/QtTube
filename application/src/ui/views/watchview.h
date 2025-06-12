#pragma once
#include "preloaddata.h"
#include "qttube-plugin/components/replytypes.h"
#include <QPointer>
#include <QWidget>

namespace Ui { class WatchView; }

class QTimer;

class WatchView : public QWidget
{
    Q_OBJECT
public:
    explicit WatchView(const QString& videoId, int progress = 0,
                       PreloadData::WatchView* preload = nullptr, QWidget* parent = nullptr);
    ~WatchView();
    void hotLoadVideo(const QString& videoId, int progress = 0, PreloadData::WatchView* preload = nullptr);
protected:
    void resizeEvent(QResizeEvent* event) override;
private:
    QPointer<QTimer> metadataUpdateTimer;
    Ui::WatchView* ui;
    QString videoId;
    QString videoUrlPrefix;

    void processPreloadData(PreloadData::WatchView* preload);
    void updateMetadata(const QString& videoId);
private slots:
    void descriptionLinkActivated(const QString& url);
    void processData(const QtTube::VideoData& data);
    void rate(bool like, const std::any& addData, const std::any& removeData);
    void showShareModal();
signals:
    void loadFailed(const QtTube::PluginException& ex);
};
