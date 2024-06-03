#include "gridvideorenderer.h"
#include "ui/widgets/labels/channellabel.h"
#include "utils/uiutils.h"
#include "videothumbnailwidget.h"
#include <QBoxLayout>

GridVideoRenderer::GridVideoRenderer(QWidget* parent) : VideoRenderer(parent), vbox(new QVBoxLayout(this))
{
    thumbnail->setPreferredSize(QSize(205, 115));

    titleLabel->setFont(QFont(font().toString(), font().pointSize() + 1, QFont::Bold));
    titleLabel->setMaximumWidth(205);
    titleLabel->setWordWrap(true);
    UIUtils::setMaximumLines(titleLabel, 2);

    channelLabel->text->setFont(QFont(font().toString(), font().pointSize() - 1));
    channelLabel->text->setWordWrap(true);
    UIUtils::setMaximumLines(channelLabel->text, 2);

    metadataLabel->setFont(QFont(font().toString(), font().pointSize() - 1));
    metadataLabel->setMaximumWidth(205);
    metadataLabel->setWordWrap(true);
    UIUtils::setMaximumLines(metadataLabel, 2);

    vbox->addWidget(thumbnail);
    vbox->addWidget(titleLabel);
    vbox->addWidget(channelLabel);
    vbox->addWidget(metadataLabel);
    vbox->addStretch();
}
