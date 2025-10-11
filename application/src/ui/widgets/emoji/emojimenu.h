#pragma once
#include "qttube-plugin/objects/emoji.h"
#include <QWidget>

namespace Ui {
class EmojiMenu;
}

class CachedNetworkWorker;
class EmojiGraphicsItem;
struct EmojiGroup;
class QGraphicsScene;
class QGraphicsTextItem;

class EmojiMenu : public QWidget
{
    Q_OBJECT
public:
    explicit EmojiMenu(QWidget* parent = nullptr, Qt::WindowFlags f = {});
    ~EmojiMenu();
protected:
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
private:
    QList<std::pair<QGraphicsTextItem*, QList<EmojiGraphicsItem*>>> m_emojiGroupItems;
    QThread* m_netThread;
    CachedNetworkWorker* m_netWorker;
    QGraphicsScene* m_scene;
    Ui::EmojiMenu* ui;

    void add(const QList<EmojiGroup>& emojiGroups);
private slots:
    void beginSearch();
    void doSceneLayout();
    void filterEmojis();
signals:
    void emojiClicked(const QtTubePlugin::Emoji& emoji);
};
