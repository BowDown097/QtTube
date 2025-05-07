#include "browsevideorenderer.h"
#include "ui/widgets/labels/channellabel.h"
#include "videothumbnailwidget.h"
#include <QBoxLayout>

BrowseVideoRenderer::BrowseVideoRenderer(QWidget* parent)
    : VideoRenderer(parent), hbox(new QHBoxLayout(this)), textVbox(new QVBoxLayout)
{
    thumbnail->setFixedSize(178, 100);
    textVbox->addWidget(titleLabel);
    textVbox->addWidget(channelLabel);
    textVbox->addWidget(metadataLabel);
    textVbox->addStretch();

    hbox->addWidget(thumbnail);
    hbox->addLayout(textVbox, 1);
}
