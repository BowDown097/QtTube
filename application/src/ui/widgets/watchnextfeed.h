#pragma once
#include "qttube-plugin/components/replytypes/recommendedcontinuationdata.h"
#include "qttube-plugin/components/replytypes/videodata.h"
#include <QTabWidget>

class ContinuableListWidget;
class PluginEntry;

class WatchNextFeed : public QTabWidget
{
public:
    explicit WatchNextFeed(PluginEntry* plugin, QWidget* parent = nullptr);
    ContinuableListWidget* currentList();
    void reset();
    void setData(
        const QString& videoId,
        const QList<QtTubePlugin::Video>& recommendedVideos,
        const QtTubePlugin::VideoData::Continuations& continuations);
private:
    ContinuableListWidget* m_commentsList;
    PluginEntry* m_plugin;
    ContinuableListWidget* m_recommendedList;
    QString m_videoId;

    void populateRecommended(const QList<QtTubePlugin::Video>& videos);
private slots:
    void continueComments();
    void continueRecommended();
    void continueRecommendedFinished(const QtTubePlugin::RecommendedContinuationData& data);
};
