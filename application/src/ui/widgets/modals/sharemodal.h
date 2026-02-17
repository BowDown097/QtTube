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
    QToolButton* m_copyButton;
    QAction* m_copyButtonAction;
    CloseButton* m_headerCloseButton;
    TubeLabel* m_headerLabel;
    QHBoxLayout* m_headerLayout;
    QVBoxLayout* m_layout;
    QLineEdit* m_lineEdit;
    QHBoxLayout* m_urlLayout;
private slots:
    void copyUrl();
};
