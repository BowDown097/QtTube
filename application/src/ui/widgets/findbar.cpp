#include "findbar.h"
#include "closebutton.h"
#include "utils/uiutils.h"
#include <QApplication>
#include <QBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QScrollArea>

constexpr QLatin1String SelectedStylesheet("background-color: yellow");

FindBar::FindBar(QWidget* parent)
    : QWidget(parent),
      m_closeButton(new CloseButton(this)),
      m_layout(new QHBoxLayout(this)),
      m_matchesLabel(new QLabel(this)),
      m_nextButton(new QPushButton(this)),
      m_previousButton(new QPushButton(this)),
      m_searchBox(new QLineEdit(this))
{
    m_layout->addWidget(m_closeButton);

    m_searchBox->setPlaceholderText("Find in program");
    m_layout->addWidget(m_searchBox);

    m_nextButton->setEnabled(false);
    m_nextButton->setText("Next");
    m_layout->addWidget(m_nextButton);

    m_previousButton->setEnabled(false);
    m_previousButton->setText("Previous");
    m_layout->addWidget(m_previousButton);

    m_layout->addWidget(m_matchesLabel);

    hide();
    setAutoFillBackground(true);
    setPalette(qApp->palette().alternateBase().color());

    connect(m_closeButton, &CloseButton::clicked, this, [this] { setReveal(false); });
    connect(m_nextButton, &QPushButton::clicked, this, &FindBar::goToNext);
    connect(m_previousButton, &QPushButton::clicked, this, &FindBar::goToPrevious);
    connect(m_searchBox, &QLineEdit::returnPressed, this, &FindBar::returnPressed);
    connect(m_searchBox, &QLineEdit::textChanged, this, &FindBar::initializeSearch);
}

void FindBar::clearMatches()
{
    for (const QPointer<QLabel>& match : std::as_const(m_matches))
        unhighlightMatch(match);
    m_matches.clear();
    m_matchesLabel->clear();
    m_currentIndex = 0;
}

void FindBar::goToNext()
{
    m_currentIndex++;
    unhighlightMatch(m_matches[m_currentIndex - 1]);
    jumpToCurrentMatch();
}

void FindBar::goToPrevious()
{
    m_currentIndex--;
    unhighlightMatch(m_matches[m_currentIndex + 1]);
    jumpToCurrentMatch();
}

void FindBar::highlightMatch(const QPointer<QLabel>& label)
{
    if (!label.isNull() && !label->styleSheet().contains(SelectedStylesheet))
        label->setStyleSheet(label->styleSheet() + SelectedStylesheet);
}

void FindBar::initializeSearch(const QString& searchText)
{
    clearMatches();
    if (searchText.isEmpty())
        return;

    const QList<QLabel*> labels = parentWidget()->findChildren<QLabel*>();
    for (QLabel* label : labels)
        if (label->isVisible() && label->text().contains(searchText, Qt::CaseInsensitive))
            m_matches.append(label);

    jumpToCurrentMatch();
}

void FindBar::jumpToCurrentMatch()
{
    m_nextButton->setEnabled(m_currentIndex + 1 < m_matches.length());
    m_previousButton->setEnabled(m_currentIndex >= 1);

    if (m_matches.empty())
    {
        m_matchesLabel->setText("Phrase not found");
        return;
    }

    highlightMatch(m_matches[m_currentIndex]);
    m_matchesLabel->setText(m_matches.length() > 1
        ? QStringLiteral("%1 of %2 matches").arg(m_currentIndex + 1).arg(m_matches.length())
        : QStringLiteral("%1 of 1 match").arg(m_currentIndex + 1));

    // if match found, but the match is deleted, try again
    if (m_matches[m_currentIndex].isNull())
    {
        initializeSearch(m_searchBox->text());
        return;
    }

    if (QListWidget* list = UIUtils::findParent<QListWidget*>(m_matches[m_currentIndex]))
    {
        for (int i = 0; i < list->count(); i++)
        {
            if (QWidget* itemWidget = list->itemWidget(list->item(i)))
            {
                if (itemWidget->findChildren<QLabel*>().contains(m_matches[m_currentIndex]))
                {
                    list->scrollToItem(list->item(i));
                    break;
                }
            }
        }
    }
    else if (QScrollArea* scrollArea = UIUtils::findParent<QScrollArea*>(m_matches[m_currentIndex]))
    {
        scrollArea->ensureWidgetVisible(m_matches[m_currentIndex]);
    }
}

void FindBar::returnPressed()
{
    if (m_matches.empty())
        return;

    if (++m_currentIndex >= m_matches.length())
    {
        m_currentIndex = 0;
        unhighlightMatch(m_matches.last());
    }
    else
    {
        unhighlightMatch(m_matches[m_currentIndex - 1]);
    }

    jumpToCurrentMatch();
}

void FindBar::setReveal(bool reveal)
{
    if (reveal)
    {
        move(0, parentWidget()->height() - 50);
        resize(parentWidget()->width(), 50);
        show();
        m_searchBox->setFocus();
    }
    else
    {
        clearMatches();
        hide();
        m_nextButton->setEnabled(false);
        m_previousButton->setEnabled(false);
        m_matchesLabel->clear();
        m_searchBox->clear();
    }
}

void FindBar::unhighlightMatch(const QPointer<QLabel>& label)
{
    if (!label.isNull() && label->styleSheet().contains(SelectedStylesheet))
        label->setStyleSheet(label->styleSheet().remove(SelectedStylesheet));
}
