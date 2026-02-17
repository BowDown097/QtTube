#include "browsevideorenderer.h"
#include "ui/widgets/labels/channellabel.h"
#include "videothumbnailwidget.h"
#include <QBoxLayout>

BrowseVideoRenderer::BrowseVideoRenderer(PluginData* plugin, QWidget* parent)
    : VideoRenderer(plugin, parent), m_layout(new QHBoxLayout(this)), m_textLayout(new QVBoxLayout)
{
    thumbnail->setFixedSize(178, 100);
    m_textLayout->addWidget(titleLabel);
    m_textLayout->addWidget(channelLabel);
    m_textLayout->addWidget(metadataLabel);
    m_textLayout->addLayout(badgesLayout);
    m_textLayout->addStretch();

    m_layout->addWidget(thumbnail);
    m_layout->addLayout(m_textLayout, 1);
}
