#include "gridvideorenderer.h"
#include "ui/widgets/labels/channellabel.h"
#include "videothumbnailwidget.h"
#include <QBoxLayout>

constexpr int EntryWidth = 205;

GridVideoRenderer::GridVideoRenderer(PluginEntry* plugin, QWidget* parent)
    : VideoRenderer(plugin, parent), m_layout(new QVBoxLayout(this))
{
    setFixedWidth(210);
    thumbnail->setFixedSize(EntryWidth, 115);

    titleLabel->setFont(QFont(font().toString(), font().pointSize() + 1, QFont::Bold));
    titleLabel->setMaximumLines(2);
    titleLabel->setMaximumWidth(EntryWidth);
    titleLabel->setWordWrap(true);

    channelLabel->text->setElideMode(Qt::ElideRight);
    channelLabel->text->setFont(QFont(font().toString(), font().pointSize() - 1));
    channelLabel->text->setMaximumLines(2);
    channelLabel->text->setMaximumWidth(EntryWidth);
    channelLabel->text->setWordWrap(true);

    metadataLabel->setElideMode(Qt::ElideRight);
    metadataLabel->setFont(QFont(font().toString(), font().pointSize() - 1));
    metadataLabel->setMaximumLines(2);
    metadataLabel->setMaximumWidth(EntryWidth);
    metadataLabel->setWordWrap(true);

    m_layout->addWidget(thumbnail);
    m_layout->addWidget(titleLabel);
    m_layout->addWidget(channelLabel);
    m_layout->addWidget(metadataLabel);
    m_layout->addLayout(badgesLayout);
    m_layout->addStretch();
}
