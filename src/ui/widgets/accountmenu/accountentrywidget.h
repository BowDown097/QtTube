#pragma once
#include <QWidget>

struct CredentialSet;
class HttpReply;
class QHBoxLayout;
class QLabel;

class AccountEntryWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AccountEntryWidget(const CredentialSet& credSet, QWidget* parent = nullptr);
    void setClickable(bool clickable) { this->clickable = clickable; }
protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent*) override;
#else
    void enterEvent(QEvent*) override;
#endif
    void leaveEvent(QEvent*) override;
    void mousePressEvent(QMouseEvent* event) override;
private slots:
    void setAvatar(const HttpReply& reply);
signals:
    void clicked();
private:
    QLabel* avatarLabel;
    bool clickable = true;
    QHBoxLayout* layout;
    QLabel* nameLabel;
};
