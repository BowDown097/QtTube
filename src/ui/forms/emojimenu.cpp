#include "emojimenu.h"
#include "ui_emojimenu.h"
#include "http.h"
#include "ui/widgets/flowlayout.h"
#include "ui/widgets/labels/tubelabel.h"
#include "ytemoji.h"

EmojiMenu::EmojiMenu(QWidget* parent) : QWidget(parent), ui(new Ui::EmojiMenu)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);
    connect(ui->emojiSearch, &QLineEdit::textEdited, this, &EmojiMenu::filterEmojis);

    layout = new FlowLayout(ui->scrollAreaContents);

    for (const ytemoji::YouTubeEmoji& ytEmoji : ytemoji::instance()->youtubeEmojis())
    {
        TubeLabel* emoji = new TubeLabel(ui->scrollAreaContents);
        emoji->setClickable(true, false);
        emoji->setToolTip(ytEmoji.shortcut);
        layout->addWidget(emoji);
        connect(emoji, &TubeLabel::clicked, this, std::bind(&EmojiMenu::emojiClicked, this, ytEmoji.shortcut));

        QUrl url(ytEmoji.image);
        HttpReply* reply = Http::instance().get(url);
        connect(reply, &HttpReply::finished, this, std::bind(&EmojiMenu::setEmojiIcon, this, std::placeholders::_1, emoji));
    }

    for (const ytemoji::UnicodeEmoji& uniEmoji : ytemoji::instance()->unicodeEmojis())
    {
        // TODO: implement shortcut-less emojis (i think just skin tone ones)
        if (uniEmoji.shortcuts.isEmpty())
            continue;

        TubeLabel* emoji = new TubeLabel(ui->scrollAreaContents);
        emoji->setClickable(true, false);
        emoji->setToolTip(uniEmoji.shortcuts[0]);
        layout->addWidget(emoji);
        connect(emoji, &TubeLabel::clicked, this, std::bind(&EmojiMenu::emojiClicked, this, uniEmoji.shortcuts[0]));

        QUrl url(uniEmoji.image);
        HttpReply* reply = Http::instance().get(url);
        connect(reply, &HttpReply::finished, this, std::bind(&EmojiMenu::setEmojiIcon, this, std::placeholders::_1, emoji));
    }
}

EmojiMenu::~EmojiMenu()
{
    delete ui;
}

void EmojiMenu::filterEmojis()
{
    // TODO: use youtube's search terms instead of label tooltip
    bool searchingEmojis = !ui->emojiSearch->text().isEmpty();
    if (searchingEmojis)
    {
        for (TubeLabel* label : ui->scrollAreaContents->findChildren<TubeLabel*>())
            label->setVisible(label->toolTip().contains(ui->emojiSearch->text()));
    }
    else
    {
        for (TubeLabel* label : ui->scrollAreaContents->findChildren<TubeLabel*>())
            label->setVisible(true);
    }
}

void EmojiMenu::setEmojiIcon(const HttpReply& reply, TubeLabel* emojiLabel)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    emojiLabel->setPixmap(pixmap.scaled(24, 24, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}
