#include "gridvideorenderer.h"
#include "ui/widgets/labels/channellabel.h"
#include "ui/widgets/labels/elidedtubelabel.h"
#include "utils/uiutils.h"
#include "videothumbnailwidget.h"
#include <QApplication>
#include <QBoxLayout>

GridVideoRenderer::GridVideoRenderer(QWidget* parent) : VideoRenderer(parent), vbox(new QVBoxLayout(this))
{
    setThumbnailSize(QSize(205, 115));

    titleLabel->setFont(QFont(qApp->font().toString(), qApp->font().pointSize() + 1, QFont::Bold));
    titleLabel->setWordWrap(true);
    UIUtils::setMaximumLines(titleLabel, 2);

    channelLabel->text->setFont(QFont(qApp->font().toString(), qApp->font().pointSize() - 1));
    channelLabel->text->setWordWrap(true);
    UIUtils::setMaximumLines(channelLabel->text, 2);

    metadataLabel->setFont(QFont(qApp->font().toString(), qApp->font().pointSize() - 1));
    metadataLabel->setWordWrap(true);
    UIUtils::setMaximumLines(metadataLabel, 2);

    vbox->addWidget(thumbnail);
    vbox->addWidget(titleLabel);
    vbox->addWidget(channelLabel);
    vbox->addWidget(metadataLabel);
    vbox->addStretch();
}
