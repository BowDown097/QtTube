#include "sharemodal.h"
#include "ui/widgets/closebutton.h"
#include "ui/widgets/labels/iconlabel.h"
#include "ui/widgets/labels/tubelabel.h"
#include "utils/uiutils.h"
#include <QBoxLayout>
#include <QLineEdit>

ShareModal::ShareModal(const QString& videoUrlPrefix, const QString& videoId, QWidget* parent)
    : FocusedModal(parent),
      copyButton(new QToolButton),
      copyButtonAction(new QAction),
      headerCloseButton(new CloseButton),
      headerLabel(new TubeLabel("Share")),
      headerLayout(new QHBoxLayout),
      layout(new QVBoxLayout(this)),
      lineEdit(new QLineEdit(videoUrlPrefix + videoId)),
      urlLayout(new QHBoxLayout)
{
    setFixedSize(400, 100);

    copyButtonAction->setIcon(UIUtils::iconThemed("link"));
    copyButton->setDefaultAction(copyButtonAction);
    copyButton->setToolTip("Copy");

    headerLayout->addWidget(headerLabel, 0, Qt::AlignLeft);
    headerLayout->addWidget(headerCloseButton, 0, Qt::AlignRight);
    layout->addLayout(headerLayout);

    urlLayout->addWidget(lineEdit);
    urlLayout->addWidget(copyButton);
    layout->addLayout(urlLayout);

    reorient();

    connect(copyButtonAction, &QAction::triggered, this, &ShareModal::copyUrl);
    connect(headerCloseButton, &CloseButton::clicked, this, &ShareModal::close);
}

void ShareModal::copyUrl()
{
    UIUtils::copyToClipboard(lineEdit->text());
}
