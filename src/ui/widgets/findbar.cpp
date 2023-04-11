#include "findbar.h"
#include "ui/uiutilities.h"
#include <QApplication>
#include <QListWidget>

FindBar::FindBar(QWidget* parent) : QWidget(parent)
{
    hbox = new QHBoxLayout(this);

    searchBox = new QLineEdit(this);
    searchBox->setPlaceholderText("Find in program");
    hbox->addWidget(searchBox);

    nextButton = new QPushButton(this);
    nextButton->setEnabled(false);
    nextButton->setText("Next");
    hbox->addWidget(nextButton);

    previousButton = new QPushButton(this);
    previousButton->setEnabled(false);
    previousButton->setText("Previous");
    hbox->addWidget(previousButton);

    matchesLabel = new QLabel(this);
    hbox->addWidget(matchesLabel);

    setAutoFillBackground(true);
    setLayout(hbox);
    setPalette(qApp->palette().alternateBase().color());
    setVisible(false);

    connect(nextButton, &QPushButton::clicked, this, &FindBar::goToNext);
    connect(previousButton, &QPushButton::clicked, this, &FindBar::goToPrevious);
    connect(searchBox, &QLineEdit::textChanged, this, &FindBar::initializeSearch);
}

void FindBar::clearLabels()
{
    for (QLabel* label : matches)
    {
        if (!label->styleSheet().isEmpty())
            label->setStyleSheet(QString());
    }

    matches.clear();
    currentIndex = 0;
}

void FindBar::goToNext()
{
    currentIndex++;
    jumpToLabel();
    matches[currentIndex - 1]->setStyleSheet(QString());
}

void FindBar::goToPrevious()
{
    currentIndex--;
    jumpToLabel();
    matches[currentIndex + 1]->setStyleSheet(QString());
}

void FindBar::initializeSearch(const QString& searchText)
{
    clearLabels();

    if (!searchText.isEmpty())
    {
        for (QLabel* label : parentWidget()->findChildren<QLabel*>())
        {
            if (label->text().contains(searchText, Qt::CaseInsensitive))
                matches.append(label);
        }
    }

    jumpToLabel();
}

void FindBar::jumpToLabel()
{
    nextButton->setEnabled(currentIndex + 1 < matches.length());
    previousButton->setEnabled(currentIndex >= 1);
    if (matches.length() > 0)
    {
        matches[currentIndex]->setStyleSheet("background-color: yellow");
        matchesLabel->setText(matches.length() > 1
                              ? QStringLiteral("%1 of %2 matches").arg(currentIndex + 1).arg(matches.length())
                              : QStringLiteral("%1 of 1 match").arg(currentIndex + 1));

        if (QListWidget* list = UIUtilities::findParent<QListWidget*>(matches[currentIndex]))
        {
            for (int i = 0; i < list->count(); i++)
            {
                if (QWidget* itemWidget = list->itemWidget(list->item(i)))
                {
                    QList<QLabel*> labels = itemWidget->findChildren<QLabel*>();
                    bool foundMatch = std::any_of(labels.begin(), labels.end(), [this](const QLabel* label)
                                                  { return label == matches[currentIndex]; });
                    if (foundMatch)
                    {
                        list->scrollToItem(list->item(i));
                        break;
                    }
                }
            }
        }
    }
    else
    {
        matchesLabel->setText("Phrase not found");
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
