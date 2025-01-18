#include "backstagequizexplanation.h"
#include "innertube/objects/backstage/quiz/quiz.h"
#include "ui/widgets/labels/tubelabel.h"
#include "utils/uiutils.h"
#include <QBoxLayout>

BackstageQuizExplanation::BackstageQuizExplanation(QWidget* parent)
    : QWidget(parent),
      m_content(new TubeLabel(this)),
      m_header(new TubeLabel(this)),
      m_layout(new QVBoxLayout(this)),
      m_readMoreLabel(new TubeLabel(this))
{
    m_header->setFont(QFont(font().toString(), -1, QFont::Bold));
    m_layout->addWidget(m_header);

    m_content->setElideMode(Qt::TextElideMode::ElideRight);
    m_content->setTextFormat(Qt::RichText);
    m_content->setWordWrap(true);
    UIUtils::setMaximumLines(m_content, 3);
    m_layout->addWidget(m_content);

    m_readMoreLabel->setClickable(true);
    m_readMoreLabel->setFont(QFont(font().toString(), -1, QFont::Bold));
    m_layout->addWidget(m_readMoreLabel);

    connect(m_readMoreLabel, &TubeLabel::clicked, this, &BackstageQuizExplanation::toggleReadMore);
}

void BackstageQuizExplanation::setData(const InnertubeObjects::Quiz& quiz, const QString& explanation)
{
    m_readMoreText = quiz.explanationExpandText.text;
    m_showLessText = quiz.explanationCollapseText.text;

    m_content->setText(explanation);
    m_header->setText(quiz.explanationHeaderText.text);
    m_readMoreLabel->setText(quiz.explanationExpandText.text);
    m_readMoreLabel->setVisible(m_content->heightForWidth(width()) > m_content->maximumHeight());
}

void BackstageQuizExplanation::toggleReadMore()
{
    if (m_readMoreLabel->text() == m_readMoreText)
    {
        m_content->setMaximumHeight(QWIDGETSIZE_MAX);
        m_readMoreLabel->setText(m_showLessText);
    }
    else
    {
        UIUtils::setMaximumLines(m_content, 3);
        m_readMoreLabel->setText(m_readMoreText);
    }

    adjustSize();
    emit readMoreClicked();
}
