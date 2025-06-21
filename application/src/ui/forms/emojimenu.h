#pragma once
#include "qttube-plugin/objects/emoji.h"
#include <QWidget>

namespace Ui {
class EmojiMenu;
}

struct EmojiGroup;
class EmojiLabel;
class FlowLayout;
class TubeLabel;

class EmojiMenu : public QWidget
{
    Q_OBJECT
public:
    explicit EmojiMenu(QWidget *parent = nullptr);
    ~EmojiMenu();
private:
    QList<std::pair<TubeLabel*, QList<EmojiLabel*>>> emojiGroupLabels;
    FlowLayout* layout;
    Ui::EmojiMenu* ui;

    void add(const QList<EmojiGroup>& emojiGroups);
private slots:
    void beginSearch();
    void filterEmojis();
signals:
    void emojiClicked(const QtTube::Emoji& emoji);
};
