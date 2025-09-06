#pragma once
#include "qttube-plugin/components/replytypes/recommendedcontinuationdata.h"
#include "qttube-plugin/components/replytypes/videodata.h"
#include <QTabWidget>

class ContinuableListWidget;
struct PluginData;

class WatchNextFeed : public QTabWidget
{
public:
    explicit WatchNextFeed(PluginData* plugin, QWidget* parent = nullptr);
    void reset();
    void setData(
        const QString& videoId,
        const QList<QtTubePlugin::Video>& recommendedVideos,
        const QtTubePlugin::VideoData::Continuations& continuations);

    ContinuableListWidget* currentList() { return qobject_cast<ContinuableListWidget*>(currentWidget()); }
private:
    ContinuableListWidget* comments;
    std::any commentsContinuation;
    PluginData* plugin;
    ContinuableListWidget* recommended;
    QString videoId;

    void populateRecommended(const QList<QtTubePlugin::Video>& videos);
private slots:
    void continueComments();
    void continueRecommended();
    void continueRecommendedFinished(const QtTubePlugin::RecommendedContinuationData& data);
};
