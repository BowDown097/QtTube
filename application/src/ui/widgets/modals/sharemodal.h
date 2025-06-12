#pragma once
#include "focusedmodal.h"

class CloseButton;
class QHBoxLayout;
class QLineEdit;
class QToolButton;
class QVBoxLayout;
class TubeLabel;

class ShareModal : public FocusedModal
{
    Q_OBJECT
public:
    ShareModal(const QString& videoUrlPrefix, const QString& videoId, QWidget* parent);
private:
    QToolButton* copyButton;
    QAction* copyButtonAction;
    CloseButton* headerCloseButton;
    TubeLabel* headerLabel;
    QHBoxLayout* headerLayout;
    QVBoxLayout* layout;
    QLineEdit* lineEdit;
    QHBoxLayout* urlLayout;
private slots:
    void copyUrl();
};
