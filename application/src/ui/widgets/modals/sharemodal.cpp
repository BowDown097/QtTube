#include "sharemodal.h"
#include "ui/widgets/closebutton.h"
#include "ui/widgets/labels/iconlabel.h"
#include "ui/widgets/labels/tubelabel.h"
#include "utils/uiutils.h"
#include <QBoxLayout>
#include <QLineEdit>

ShareModal::ShareModal(const QString& videoUrlPrefix, const QString& videoId, QWidget* parent)
    : FocusedModal(parent),
      m_copyButton(new QToolButton),
      m_copyButtonAction(new QAction),
      m_headerCloseButton(new CloseButton),
      m_headerLabel(new TubeLabel("Share")),
      m_headerLayout(new QHBoxLayout),
      m_layout(new QVBoxLayout(this)),
      m_lineEdit(new QLineEdit(videoUrlPrefix + videoId)),
      m_urlLayout(new QHBoxLayout)
{
    setFixedSize(400, 100);

    m_copyButtonAction->setIcon(UIUtils::iconThemed("link"));
    m_copyButton->setDefaultAction(m_copyButtonAction);
    m_copyButton->setToolTip("Copy");

    m_headerLayout->addWidget(m_headerLabel, 0, Qt::AlignLeft);
    m_headerLayout->addWidget(m_headerCloseButton, 0, Qt::AlignRight);
    m_layout->addLayout(m_headerLayout);

    m_urlLayout->addWidget(m_lineEdit);
    m_urlLayout->addWidget(m_copyButton);
    m_layout->addLayout(m_urlLayout);

    reorient();

    connect(m_copyButtonAction, &QAction::triggered, this, &ShareModal::copyUrl);
    connect(m_headerCloseButton, &CloseButton::clicked, this, &ShareModal::close);
}

void ShareModal::copyUrl()
{
    UIUtils::copyToClipboard(m_lineEdit->text());
}
