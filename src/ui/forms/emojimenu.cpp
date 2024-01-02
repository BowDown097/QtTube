#include "emojimenu.h"
#include "ui_emojimenu.h"
#include "ui/widgets/flowlayout.h"
#include "ui/widgets/labels/emojilabel.h"
#include "ytemoji.h"

EmojiMenu::EmojiMenu(QWidget* parent) : QWidget(parent), ui(new Ui::EmojiMenu)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);
    connect(ui->emojiSearch, &QLineEdit::textEdited, this, &EmojiMenu::filterEmojis);

    layout = new FlowLayout(ui->scrollAreaContents);

    for (const ytemoji::YouTubeEmoji& ytEmoji : ytemoji::instance()->youtubeEmojis())
    {
        EmojiLabel* emoji = new EmojiLabel(ytEmoji.shortcut, ytEmoji.image, ui->scrollAreaContents);
        layout->addWidget(emoji);
        connect(emoji, &EmojiLabel::clicked, this, std::bind(&EmojiMenu::emojiClicked, this, emoji->primaryShortcut()));
    }

    for (const ytemoji::UnicodeEmoji& uniEmoji : ytemoji::instance()->unicodeEmojis())
    {
        // TODO: implement shortcut-less emojis (i think just skin tone ones)
        if (uniEmoji.shortcuts.isEmpty())
            continue;

        EmojiLabel* emoji = new EmojiLabel(uniEmoji.shortcuts, uniEmoji.searchTerms, uniEmoji.image, ui->scrollAreaContents);
        layout->addWidget(emoji);
        connect(emoji, &EmojiLabel::clicked, this, std::bind(&EmojiMenu::emojiClicked, this, emoji->primaryShortcut()));
    }
}

EmojiMenu::~EmojiMenu()
{
    delete ui;
}

void EmojiMenu::filterEmojis()
{
    const QString searchText = ui->emojiSearch->text();
    if (!searchText.isEmpty())
    {
        for (EmojiLabel* label : ui->scrollAreaContents->findChildren<EmojiLabel*>())
        {
            const QStringList searchTerms = label->searchTerms();
            label->setVisible(std::ranges::any_of(searchTerms, [label, &searchText](const QString& searchTerm) {
                return searchTerm.contains(searchText);
            }));
        }
    }
    else
    {
        for (EmojiLabel* label : ui->scrollAreaContents->findChildren<EmojiLabel*>())
            label->setVisible(true);
    }
}
