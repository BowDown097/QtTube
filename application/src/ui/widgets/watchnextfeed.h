#pragma once
#include "qttube-plugin/objects/videodata.h"
#include <QTabWidget>

class ContinuableListWidget;

class WatchNextFeed : public QTabWidget
{
public:
    explicit WatchNextFeed(QWidget* parent = nullptr);
    void reset();
    void setData(
        const QList<QtTube::PluginVideo>& recommendedVideos,
        const QtTube::VideoData::Continuations& continuations);

    ContinuableListWidget* currentList() { return qobject_cast<ContinuableListWidget*>(currentWidget()); }
private:
    ContinuableListWidget* comments;
    std::any commentsContinuation;
    ContinuableListWidget* recommended;
    std::any recommendedContinuation;
private slots:
    void continueComments();
    void continueRecommended();
};
