#include "emojimenu.h"
#include "ui_emojimenu.h"
#include "http.h"
#include "ytemoji.h"

int codepoint(const std::string &u)
{
    int l = u.length();
    if (l<1) return -1; unsigned char u0 = u[0]; if (u0<=127) return u0;
    if (l<2) return -1; unsigned char u1 = u[1]; if (u0>=192 && u0<=223) return (u0-192)*64 + (u1-128);
    if ((u[0] & 0xed) == 0xed && (u[1] & 0xa0) == 0xa0) return -1; //code points, 0xd800 to 0xdfff
    if (l<3) return -1; unsigned char u2 = u[2]; if (u0>=224 && u0<=239) return (u0-224)*4096 + (u1-128)*64 + (u2-128);
    if (l<4) return -1; unsigned char u3 = u[3]; if (u0>=240 && u0<=247) return (u0-240)*262144 + (u1-128)*4096 + (u2-128)*64 + (u3-128);
    return -1;
}

EmojiMenu::EmojiMenu(QWidget *parent) : QWidget(parent), ui(new Ui::EmojiMenu)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);
    ui->filteredScrollArea->setVisible(false);
    connect(ui->emojiSearch, &QLineEdit::textEdited, this, &EmojiMenu::filterEmojis);

    filteredLayout = new FlowLayout(ui->filteredScrollAreaContents);
    layout = new FlowLayout(ui->scrollAreaContents);

    for (const ytemoji::YouTubeEmoji& ytEmoji : ytemoji::YOUTUBE_EMOJIS)
    {
        TubeLabel* emoji = new TubeLabel(ui->scrollAreaContents);
        emoji->setClickable(true, false);
        emoji->setToolTip(ytEmoji.emojiName);
        layout->addWidget(emoji);
        connect(emoji, &TubeLabel::clicked, this, [this, ytEmoji] { emit emojiClicked(ytEmoji.emojiName); });

        QUrl url("https://yt3.ggpht.com/" + ytEmoji.emojiPath);
        HttpReply* reply = Http::instance().get(url);
        connect(reply, &HttpReply::finished, this, [this, emoji](const HttpReply& reply) { setEmojiIcon(reply, emoji); });
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 1, 0) && defined(__cpp_lib_char8_t)
    for (QMap<QString, std::u8string>::const_iterator it = ytemoji::BUILTIN_EMOJIS.begin(); it != ytemoji::BUILTIN_EMOJIS.end(); ++it)
#else
    for (QMap<QString, QString>::const_iterator it = ytemoji::BUILTIN_EMOJIS.begin(); it != ytemoji::BUILTIN_EMOJIS.end(); ++it)
#endif
    {
#if QT_VERSION < QT_VERSION_CHECK(6, 1, 0) && defined(__cpp_lib_char8_t)
        QString codepointHex = QString::number(codepoint(std::string(it.value().begin(), it.value().end())), 16);
#else
        QString codepointHex = QString::number(codepoint(it.value().toStdString()), 16);
#endif

        TubeLabel* emoji = new TubeLabel(ui->scrollAreaContents);
        emoji->setClickable(true, false);
        emoji->setToolTip(it.key());
        layout->addWidget(emoji);
        connect(emoji, &TubeLabel::clicked, this, [this, it] { emit emojiClicked(it.key()); });

        QUrl url(QStringLiteral("https://www.youtube.com/s/gaming/emoji/0f0cae22/emoji_u%1.svg").arg(codepointHex));
        HttpReply* reply = Http::instance().get(url);
        connect(reply, &HttpReply::finished, this, [this, emoji](const HttpReply& reply) { setEmojiIcon(reply, emoji); });
    }
}

EmojiMenu::~EmojiMenu()
{
    delete ui;
}

// WARNING: HORRIBLE DISGUSTING SPAGHETTI CODE
// Setting the visibility of the labels we don't want unfortunately doesn't work. It leaves holes in the layout.
// Therefore, I move labels between 2 layouts (the normal one and one where the filtered ones are put) and hide the layouts as necessary.
// It doesn't perform good and I really hate it. I couldn't find any way to fix this hole problem through surface level searching...
// So this was the compromise. This can be fixed... right?
void EmojiMenu::filterEmojis()
{
    for (TubeLabel* label : ui->filteredScrollAreaContents->findChildren<TubeLabel*>())
    {
        layout->addWidget(label);
    }

    if (ui->emojiSearch->text().isEmpty())
    {
        ui->filteredScrollArea->setVisible(false);
        ui->scrollArea->setVisible(true);
    }
    else
    {
        ui->filteredScrollArea->setVisible(true);
        ui->scrollArea->setVisible(false);
        for (TubeLabel* label : ui->scrollAreaContents->findChildren<TubeLabel*>())
        {
            if (label->toolTip().contains(ui->emojiSearch->text()))
                filteredLayout->addWidget(label);
        }
    }
}

void EmojiMenu::setEmojiIcon(const HttpReply& reply, TubeLabel* emojiLabel)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    emojiLabel->setPixmap(pixmap.scaled(24, 24, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}
