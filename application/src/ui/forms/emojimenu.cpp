#include "emojimenu.h"
#include "ui_emojimenu.h"
#include "stores/emojistore.h"
#include "ui/widgets/emojigraphicsitem.h"
#include <QTimer>
#include <ranges>

EmojiMenu::~EmojiMenu() { delete ui; }

EmojiMenu::EmojiMenu(QWidget* parent, Qt::WindowFlags f)
    : QWidget(parent, f), m_scene(new QGraphicsScene(this)), ui(new Ui::EmojiMenu)
{
    ui->setupUi(this);
    ui->graphicsView->setScene(m_scene);

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
        QGraphicsTextItem* groupHeader = new QGraphicsTextItem(emojiGroup.name);
        m_scene->addItem(groupHeader);

        QList<EmojiGraphicsItem*> groupEmojiItems;
        groupEmojiItems.reserve(emojiGroup.emojis.size());

        for (const QtTube::Emoji& emoji : emojiGroup.emojis)
        {
            EmojiGraphicsItem* emojiItem = new EmojiGraphicsItem(emoji);
            m_scene->addItem(emojiItem);
            connect(emojiItem, &EmojiGraphicsItem::clicked, this, [this, emojiItem] { emit emojiClicked(emojiItem->data()); });
            groupEmojiItems.append(emojiItem);
        }

        m_emojiGroupItems.emplaceBack(groupHeader, groupEmojiItems);
    }

    doSceneLayout();
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

void EmojiMenu::doSceneLayout()
{
    constexpr int emojiSize = 24;
    constexpr int spacing = 6;
    const int sceneWidth = ui->graphicsView->viewport()->width();

    int y = 0;
    for (const auto& [groupHeader, emojiItems] : std::as_const(m_emojiGroupItems))
    {
        if (!groupHeader->isVisible())
            continue;

        // put header on its own line
        groupHeader->setPos(0, y);
        y += groupHeader->boundingRect().height() + spacing;

        // put emojis in a flex layout
        int x = 0;
        for (EmojiGraphicsItem* emojiItem : emojiItems)
        {
            if (!emojiItem->isVisible())
                continue;

            // wrap to next line if needed
            if (x + emojiSize > sceneWidth)
            {
                x = 0;
                y += emojiSize + spacing;
            }

            emojiItem->setPos(x, y);
            x += emojiSize + spacing;
        }

        y += emojiSize + spacing;
    }

    m_scene->setSceneRect(0, 0, sceneWidth, y);
}

void EmojiMenu::filterEmojis()
{
    const QString searchText = ui->emojiSearch->text();
    auto termFilter = [&searchText](const QString& term) { return term.contains(searchText, Qt::CaseInsensitive); };

    for (const auto& [groupHeader, emojiItems] : std::as_const(m_emojiGroupItems))
    {
        bool anyVisible = false;

        for (EmojiGraphicsItem* emojiItem : emojiItems)
        {
            bool isVisible = searchText.isEmpty() || std::ranges::any_of(std::views::join(std::array {
                std::span(emojiItem->data().emoticons),
                std::span(emojiItem->data().shortcodes)
            }), termFilter);
            emojiItem->setVisible(isVisible);
            anyVisible |= isVisible;
        }

        groupHeader->setVisible(anyVisible);
    }

    doSceneLayout();
}

void EmojiMenu::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    doSceneLayout();
}

void EmojiMenu::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    doSceneLayout();
}
