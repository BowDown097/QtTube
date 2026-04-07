#include "basepluginentry.h"
#include "ui/widgets/labels/tubelabel.h"
#include <QBoxLayout>
#include <QDesktopServices>

BasePluginEntry::BasePluginEntry(QWidget* parent)
    : QWidget(parent),
      m_authorLabel(new TubeLabel(this)),
      m_buttonsLayout(new QHBoxLayout),
      m_descriptionLabel(new TubeLabel(this)),
      m_image(new TubeLabel(this)),
      m_layout(new QVBoxLayout(this)),
      m_nameAndVersionLayout(new QHBoxLayout),
      m_nameLabel(new TubeLabel(this)),
      m_topLayout(new QHBoxLayout),
      m_topTextLayout(new QVBoxLayout),
      m_versionLabel(new TubeLabel(this))
{
    m_authorLabel->setElideMode(Qt::ElideRight);
    m_authorLabel->setFont(QFont(font().toString(), font().pointSize() - 1));

    m_descriptionLabel->setElideMode(Qt::ElideRight);
    m_descriptionLabel->setMaximumLines(2);
    m_descriptionLabel->setWordWrap(true);

    m_image->setFixedSize(48, 48);
    m_image->setScaledContents(true);

    m_nameLabel->setFont(QFont(font().toString(), font().pointSize(), QFont::Bold));
    m_versionLabel->setFont(QFont(font().toString(), font().pointSize() - 2));

    m_nameAndVersionLayout->addWidget(m_nameLabel);
    m_nameAndVersionLayout->addWidget(m_versionLabel);
    m_nameAndVersionLayout->addStretch();

    m_topTextLayout->addStretch();
    m_topTextLayout->addLayout(m_nameAndVersionLayout);
    m_topTextLayout->addWidget(m_authorLabel);
    m_topTextLayout->addStretch();

    m_topLayout->addWidget(m_image);
    m_topLayout->addLayout(m_topTextLayout, 1);

    m_layout->addLayout(m_topLayout);
    m_layout->addWidget(m_descriptionLabel);
    m_layout->addStretch();
    m_layout->addLayout(m_buttonsLayout);
}

void BasePluginEntry::setData(const PluginEntryMetadata& metadata)
{
    m_authorLabel->setText(metadata.author);
    m_descriptionLabel->setText(metadata.description);
    m_nameLabel->setText(metadata.name);
    m_versionLabel->setText(metadata.version);

    if (!metadata.image.isEmpty())
        m_image->setImage(metadata.image, TubeLabel::Cached | TubeLabel::KeepAspectRatio);

    if (!metadata.url.isEmpty())
    {
        m_nameLabel->setClickable(true);
        m_nameLabel->setUnderlineOnHover(true);
        connect(m_nameLabel, &TubeLabel::clicked, std::bind(&QDesktopServices::openUrl, QUrl(metadata.url)));
    }
}

void BasePluginEntry::setData(const QtTubePlugin::PluginMetadata& metadata)
{
    setData(PluginEntryMetadata {
        .author = metadata.author,
        .description = metadata.description,
        .image = metadata.image,
        .name = metadata.name,
        .url = metadata.url,
        .version = metadata.version
    });
}
