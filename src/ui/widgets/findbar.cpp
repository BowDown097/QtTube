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
      closeButton(new CloseButton(this)),
      hbox(new QHBoxLayout(this)),
      matchesLabel(new QLabel(this)),
      nextButton(new QPushButton(this)),
      previousButton(new QPushButton(this)),
      searchBox(new QLineEdit(this))
{
    hbox->addWidget(closeButton);

    searchBox->setPlaceholderText("Find in program");
    hbox->addWidget(searchBox);

    nextButton->setEnabled(false);
    nextButton->setText("Next");
    hbox->addWidget(nextButton);

    previousButton->setEnabled(false);
    previousButton->setText("Previous");
    hbox->addWidget(previousButton);

    hbox->addWidget(matchesLabel);

    hide();
    setAutoFillBackground(true);
    setPalette(qApp->palette().alternateBase().color());

    connect(closeButton, &CloseButton::clicked, this, [this] { setReveal(false); });
    connect(nextButton, &QPushButton::clicked, this, &FindBar::goToNext);
    connect(previousButton, &QPushButton::clicked, this, &FindBar::goToPrevious);
    connect(searchBox, &QLineEdit::returnPressed, this, &FindBar::returnPressed);
    connect(searchBox, &QLineEdit::textChanged, this, &FindBar::initializeSearch);
}

void FindBar::clearMatches()
{
    for (const QPointer<QLabel>& match : std::as_const(matches))
        unhighlightMatch(match);
    matches.clear();
    matchesLabel->clear();
    currentIndex = 0;
}

void FindBar::goToNext()
{
    currentIndex++;
    unhighlightMatch(matches[currentIndex - 1]);
    jumpToCurrentMatch();
}

void FindBar::goToPrevious()
{
    currentIndex--;
    unhighlightMatch(matches[currentIndex + 1]);
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
            matches.append(label);

    jumpToCurrentMatch();
}

void FindBar::jumpToCurrentMatch()
{
    nextButton->setEnabled(currentIndex + 1 < matches.length());
    previousButton->setEnabled(currentIndex >= 1);

    if (matches.empty())
    {
        matchesLabel->setText("Phrase not found");
        return;
    }

    highlightMatch(matches[currentIndex]);
    matchesLabel->setText(matches.length() > 1
        ? QStringLiteral("%1 of %2 matches").arg(currentIndex + 1).arg(matches.length())
        : QStringLiteral("%1 of 1 match").arg(currentIndex + 1));

    // if match found, but the match is deleted, try again
    if (matches[currentIndex].isNull())
    {
        initializeSearch(searchBox->text());
        return;
    }

    if (QListWidget* list = UIUtils::findParent<QListWidget*>(matches[currentIndex]))
    {
        for (int i = 0; i < list->count(); i++)
        {
            if (QWidget* itemWidget = list->itemWidget(list->item(i)))
            {
                if (itemWidget->findChildren<QLabel*>().contains(matches[currentIndex]))
                {
                    list->scrollToItem(list->item(i));
                    break;
                }
            }
        }
    }
    else if (QScrollArea* scrollArea = UIUtils::findParent<QScrollArea*>(matches[currentIndex]))
    {
        scrollArea->ensureWidgetVisible(matches[currentIndex]);
    }
}

void FindBar::returnPressed()
{
    if (matches.empty())
        return;

    if (++currentIndex >= matches.length())
    {
        currentIndex = 0;
        unhighlightMatch(matches.last());
    }
    else
    {
        unhighlightMatch(matches[currentIndex - 1]);
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
        searchBox->setFocus();
    }
    else
    {
        clearMatches();
        hide();
        nextButton->setEnabled(false);
        previousButton->setEnabled(false);
        matchesLabel->clear();
        searchBox->clear();
    }
}

void FindBar::unhighlightMatch(const QPointer<QLabel>& label)
{
    if (!label.isNull() && label->styleSheet().contains(SelectedStylesheet))
        label->setStyleSheet(label->styleSheet().remove(SelectedStylesheet));
}
