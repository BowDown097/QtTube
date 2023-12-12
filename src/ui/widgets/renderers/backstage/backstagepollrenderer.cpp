#include "backstagepollrenderer.h"
#include "backstagepollchoicerenderer.h"
#include "innertube/objects/backstage/poll/poll.h"
#include <QApplication>
#include <QBoxLayout>
#include <QLabel>
#include <QProgressBar>

BackstagePollRenderer::BackstagePollRenderer(QWidget* parent)
    : layout(new QVBoxLayout(this)),
      voteCount(new QLabel(this))
{
    voteCount->setFont(QFont(qApp->font().toString(), qApp->font().pointSize() - 1));
    layout->addWidget(voteCount);
}

QList<BackstagePollChoiceRenderer*> BackstagePollRenderer::choiceRenderers() const
{
    QList<BackstagePollChoiceRenderer*> list;
    list.reserve(layout->count());

    for (int i = 0; i < layout->count(); ++i)
        if (auto choice = qobject_cast<BackstagePollChoiceRenderer*>(layout->itemAt(i)->widget()))
            list.append(choice);

    return list;
}

void BackstagePollRenderer::pollChoiceClicked()
{
    BackstagePollChoiceRenderer* senderChoice = qobject_cast<BackstagePollChoiceRenderer*>(sender());
    if (senderChoice->hasStyle())
    {
        for (BackstagePollChoiceRenderer* pollChoiceRenderer : choiceRenderers())
        {
            if (!pollChoiceRenderer->hasStyle())
            {
                pollChoiceRenderer->setValue(pollChoiceRenderer->voteRatioIfNotSelected() * 100, false);
            }
        }
    }
    else if (senderChoice->value() != -1)
    {
        for (BackstagePollChoiceRenderer* pollChoiceRenderer : choiceRenderers())
        {
            pollChoiceRenderer->reset();
        }
        return;
    }
    else
    {
        for (BackstagePollChoiceRenderer* pollChoiceRenderer : choiceRenderers())
        {
            pollChoiceRenderer->setValue(pollChoiceRenderer->voteRatioIfNotSelected() * 100, false);
        }
    }

    senderChoice->setValue(senderChoice->voteRatioIfSelected() * 100, true);
}

void BackstagePollRenderer::setData(const InnertubeObjects::Poll& poll)
{
    voteCount->setText(poll.totalVotes);
    bool hasSelected = std::ranges::any_of(poll.choices, [](const InnertubeObjects::PollChoice& choice) {
        return choice.selected;
    });

    for (const InnertubeObjects::PollChoice& pollChoice : poll.choices)
    {
        BackstagePollChoiceRenderer* pollChoiceRenderer = new BackstagePollChoiceRenderer(this);
        pollChoiceRenderer->setData(pollChoice);
        pollChoiceRenderer->setFixedHeight(36);

        if (hasSelected)
            pollChoiceRenderer->setValue(pollChoice.voteRatio * 100, pollChoice.selected);

        layout->addWidget(pollChoiceRenderer);
        connect(pollChoiceRenderer, &BackstagePollChoiceRenderer::clicked, this, &BackstagePollRenderer::pollChoiceClicked);
    }
}
