#include "emojimenu.h"
#include "ui_emojimenu.h"
#include "stores/emojistore.h"
#include "ui/widgets/flowlayout.h"
#include "ui/widgets/labels/emojilabel.h"
#include <QTimer>

EmojiMenu::~EmojiMenu() { delete ui; }

EmojiMenu::EmojiMenu(QWidget* parent, Qt::WindowFlags f)
    : QWidget(parent, f), ui(new Ui::EmojiMenu)
{
    ui->setupUi(this);
    layout = new FlowLayout(ui->scrollAreaContents);

    connect(ui->emojiSearch, &QLineEdit::textEdited, this, &EmojiMenu::beginSearch);

    if (!EmojiStore::instance()->hasBuiltinEmojis())
    {
        connect(EmojiStore::instance(), &EmojiStore::gotBuiltinEmojis, this, [this] {
            add(EmojiStore::instance()->emojiGroups());
        });
    }
    else
    {
        add(EmojiStore::instance()->emojiGroups());
    }
}

void EmojiMenu::add(const QList<EmojiGroup>& emojiGroups)
{
    for (const EmojiGroup& emojiGroup : emojiGroups)
    {
        TubeLabel* groupLabel = new TubeLabel(emojiGroup.name);
        groupLabel->setProperty("onOwnLine", true);
        layout->addWidget(groupLabel);

        QList<EmojiLabel*> groupEmojiLabels;
        groupEmojiLabels.reserve(emojiGroup.emojis.size());

        for (const QtTube::Emoji& emoji : emojiGroup.emojis)
        {
            EmojiLabel* emojiLabel = new EmojiLabel(emoji);
            layout->addWidget(emojiLabel);
            connect(emojiLabel, &EmojiLabel::clicked, this, [this, emojiLabel] { emit emojiClicked(emojiLabel->data()); });
            groupEmojiLabels.append(emojiLabel);
        }

        emojiGroupLabels.emplaceBack(groupLabel, groupEmojiLabels);
    }
}

void EmojiMenu::beginSearch()
{
    static QTimer* debounceTimer = [this] {
        QTimer* timer = new QTimer(this);
        timer->setSingleShot(true);
        timer->setInterval(150);
        connect(timer, &QTimer::timeout, this, &EmojiMenu::filterEmojis);
        return timer;
    }();

    debounceTimer->start();
}

void EmojiMenu::filterEmojis()
{
    const QString searchText = ui->emojiSearch->text();
    auto termFilter = [&searchText](const QString& term) { return term.contains(searchText, Qt::CaseInsensitive); };

    for (const auto& [groupLabel, emojiLabels] : std::as_const(emojiGroupLabels))
    {
        bool anyVisible = false;

        for (EmojiLabel* emojiLabel : std::as_const(emojiLabels))
        {
            bool isVisible = searchText.isEmpty() || std::ranges::any_of(emojiLabel->searchTerms(), termFilter);
            emojiLabel->setVisible(isVisible);
            anyVisible |= isVisible;
        }

        groupLabel->setVisible(anyVisible);
    }
}
