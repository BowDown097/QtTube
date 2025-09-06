#pragma once
#include "preloaddata.h"
#include "qttube-plugin/components/reply.h"
#include "qttube-plugin/components/replytypes/videodata.h"
#include <QPointer>
#include <QWidget>

namespace Ui { class WatchView; }

struct PluginData;
class QTimer;

class WatchView : public QWidget
{
    Q_OBJECT
public:
    explicit WatchView(const QString& videoId, PluginData* plugin, int progress = 0,
                       PreloadData::WatchView* preload = nullptr, QWidget* parent = nullptr);
    ~WatchView();
    void hotLoadVideo(
        const QString& videoId, int progress = 0,
        PreloadData::WatchView* preload = nullptr, bool continuePlayback = false);
protected:
    void resizeEvent(QResizeEvent* event) override;
private:
    QPointer<QTimer> metadataUpdateTimer;
    PluginData* plugin;
    Ui::WatchView* ui;
    QString videoId;
    QString videoUrlPrefix;

    void openLiveChat(const QtTubePlugin::InitialLiveChatData& data);
    void processPreloadData(PreloadData::WatchView* preload);
    void updateMetadata(const QString& videoId);
private slots:
    void descriptionLinkActivated(const QString& link);
    void processData(const QtTubePlugin::VideoData& data);
    void rate(bool like, const std::any& addData, const std::any& removeData);
    void showShareModal();
signals:
    void loadFailed(const QtTubePlugin::Exception& ex);
};
