#ifndef EMOJIMENU_H
#define EMOJIMENU_H
#include "httpreply.h"
#include "ui/widgets/flowlayout.h"
#include "ui/widgets/labels/tubelabel.h"
#include <QWidget>

namespace Ui {
class EmojiMenu;
}

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
    FlowLayout* filteredLayout;
    FlowLayout* layout;
    Ui::EmojiMenu *ui;
};

#endif // EMOJIMENU_H
