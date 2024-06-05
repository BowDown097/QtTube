#include "findbar.h"
#include "utils/uiutils.h"
#include <QApplication>
#include <QBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>

constexpr QLatin1String SelectedStylesheet("background-color: yellow");

FindBar::FindBar(QWidget* parent)
    : QWidget(parent),
      hbox(new QHBoxLayout(this)),
      matchesLabel(new QLabel(this)),
      nextButton(new QPushButton(this)),
      previousButton(new QPushButton(this)),
      searchBox(new QLineEdit(this))
{
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

    connect(nextButton, &QPushButton::clicked, this, &FindBar::goToNext);
    connect(previousButton, &QPushButton::clicked, this, &FindBar::goToPrevious);
    connect(searchBox, &QLineEdit::textChanged, this, &FindBar::initializeSearch);
}

void FindBar::clearLabels()
{
    for (QLabel* label : matches)
        if (label->styleSheet().contains(SelectedStylesheet))
            label->setStyleSheet(label->styleSheet().remove(SelectedStylesheet));

    matches.clear();
    currentIndex = 0;
}

void FindBar::goToNext()
{
    currentIndex++;
    jumpToLabel();
    matches[currentIndex - 1]->setStyleSheet(matches[currentIndex - 1]->styleSheet().remove(SelectedStylesheet));
}

void FindBar::goToPrevious()
{
    currentIndex--;
    jumpToLabel();
    matches[currentIndex + 1]->setStyleSheet(matches[currentIndex + 1]->styleSheet().remove(SelectedStylesheet));
}

void FindBar::initializeSearch(const QString& searchText)
{
    clearLabels();
    if (searchText.isEmpty())
        return;

    for (QLabel* label : parentWidget()->findChildren<QLabel*>())
        if (label->isVisible() && label->text().contains(searchText, Qt::CaseInsensitive))
            matches.append(label);

    jumpToLabel();
}

void FindBar::jumpToLabel()
{
    nextButton->setEnabled(currentIndex + 1 < matches.length());
    previousButton->setEnabled(currentIndex >= 1);

    if (matches.empty())
    {
        matchesLabel->setText("Phrase not found");
        return;
    }

    if (!matches[currentIndex]->styleSheet().contains(SelectedStylesheet))
        matches[currentIndex]->setStyleSheet(matches[currentIndex]->styleSheet() + SelectedStylesheet);

    matchesLabel->setText(matches.length() > 1
                              ? QStringLiteral("%1 of %2 matches").arg(currentIndex + 1).arg(matches.length())
                              : QStringLiteral("%1 of 1 match").arg(currentIndex + 1));

    if (QListWidget* list = UIUtils::findParent<QListWidget*>(matches[currentIndex]))
    {
        for (int i = 0; i < list->count(); i++)
        {
            if (QWidget* itemWidget = list->itemWidget(list->item(i)))
            {
                const QList<QLabel*> labels = itemWidget->findChildren<QLabel*>();
                bool foundMatch = std::ranges::any_of(labels, [this](QLabel* l) { return l == matches[currentIndex]; });
                if (foundMatch)
                {
                    list->scrollToItem(list->item(i));
                    break;
                }
            }
        }
    }
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
        clearLabels();
        hide();
        nextButton->setEnabled(false);
        previousButton->setEnabled(false);
        searchBox->clear();
    }
}
