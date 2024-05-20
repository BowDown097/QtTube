#pragma once
#include <QWidget>

namespace Ui {
class EmojiMenu;
}

class FlowLayout;
class HttpReply;
class TubeLabel;

class EmojiMenu : public QWidget
{
    Q_OBJECT
public:
    explicit EmojiMenu(QWidget *parent = nullptr);
    ~EmojiMenu();
private slots:
    void filterEmojis();
signals:
    void emojiClicked(const QString& emoji);
private:
    FlowLayout* layout;
    Ui::EmojiMenu* ui;
};
