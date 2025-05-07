#pragma once
#include <QTabWidget>

namespace InnertubeEndpoints { struct Next; }

class ContinuableListWidget;

class WatchNextFeed : public QTabWidget
{
public:
    explicit WatchNextFeed(QWidget* parent = nullptr);
    void reset();
    void setData(const InnertubeEndpoints::Next& endpoint);

    ContinuableListWidget* currentList() { return qobject_cast<ContinuableListWidget*>(currentWidget()); }
private:
    ContinuableListWidget* comments;
    QString commentsContinuation;
    ContinuableListWidget* recommended;
    QString recommendedContinuation;
private slots:
    void continueComments();
    void continueRecommended();
};
