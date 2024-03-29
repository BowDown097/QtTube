#ifndef BACKSTAGEPOLLRENDERER_H
#define BACKSTAGEPOLLRENDERER_H
#include <QWidget>

class BackstagePollChoiceRenderer;
class QLabel;
class QVBoxLayout;

namespace InnertubeObjects { class Poll; }

class BackstagePollRenderer : public QWidget
{
    Q_OBJECT
public:
    explicit BackstagePollRenderer(QWidget* parent = nullptr);
    QList<BackstagePollChoiceRenderer*> choiceRenderers() const;
    void setData(const InnertubeObjects::Poll& poll);
private:
    QVBoxLayout* layout;
    QLabel* voteCount;
private slots:
    void pollChoiceClicked();
};

#endif // BACKSTAGEPOLLRENDERER_H
