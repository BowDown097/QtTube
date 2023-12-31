#ifndef EMOJIMENU_H
#define EMOJIMENU_H
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
    void setEmojiIcon(const HttpReply& reply, TubeLabel* emojiLabel);
signals:
    void emojiClicked(const QString& emoji);
private:
    FlowLayout* layout;
    Ui::EmojiMenu* ui;
};

#endif // EMOJIMENU_H
