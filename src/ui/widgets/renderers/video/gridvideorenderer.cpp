#include "gridvideorenderer.h"
#include "ui/widgets/labels/channellabel.h"
#include "videothumbnailwidget.h"
#include <QBoxLayout>

GridVideoRenderer::GridVideoRenderer(QWidget* parent) : VideoRenderer(parent), vbox(new QVBoxLayout(this))
{
    thumbnail->setFixedSize(205, 115);

    titleLabel->setFont(QFont(font().toString(), font().pointSize() + 1, QFont::Bold));
    titleLabel->setMaximumLines(2);
    titleLabel->setMaximumWidth(205);
    titleLabel->setWordWrap(true);

    channelLabel->text->setFont(QFont(font().toString(), font().pointSize() - 1));
    channelLabel->text->setMaximumLines(2);
    channelLabel->text->setWordWrap(true);

    metadataLabel->setElideMode(Qt::TextElideMode::ElideRight);
    metadataLabel->setFont(QFont(font().toString(), font().pointSize() - 1));
    metadataLabel->setMaximumLines(2);
    metadataLabel->setMaximumWidth(205);
    metadataLabel->setWordWrap(true);

    vbox->addWidget(thumbnail);
    vbox->addWidget(titleLabel);
    vbox->addWidget(channelLabel);
    vbox->addWidget(metadataLabel);
    vbox->addStretch();
}
