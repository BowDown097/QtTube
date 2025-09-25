#include "basepluginentry.h"
#include "ui/widgets/labels/tubelabel.h"
#include <QBoxLayout>
#include <QDesktopServices>

BasePluginEntry::BasePluginEntry(QWidget* parent)
    : QWidget(parent),
      m_authorLabel(new TubeLabel(this)),
      m_buttonsLayout(new QHBoxLayout),
      m_descriptionLabel(new TubeLabel(this)),
      m_imageLabel(new TubeLabel(this)),
      m_layout(new QVBoxLayout(this)),
      m_nameAndAuthorLayout(new QVBoxLayout),
      m_nameLabel(new TubeLabel(this)),
      m_topLayout(new QHBoxLayout)
{
    m_authorLabel->setElideMode(Qt::ElideRight);
    m_authorLabel->setFont(QFont(m_authorLabel->font().toString(), m_authorLabel->font().pointSize() - 1));

    m_descriptionLabel->setElideMode(Qt::ElideRight);
    m_descriptionLabel->setMaximumLines(2);
    m_descriptionLabel->setWordWrap(true);

    m_imageLabel->setFixedSize(48, 48);
    m_imageLabel->setScaledContents(true);

    m_nameLabel->setElideMode(Qt::ElideRight);
    m_nameLabel->setFont(QFont(m_nameLabel->font().toString(), m_nameLabel->font().pointSize(), QFont::Bold));

    m_nameAndAuthorLayout->addStretch();
    m_nameAndAuthorLayout->addWidget(m_nameLabel);
    m_nameAndAuthorLayout->addWidget(m_authorLabel);
    m_nameAndAuthorLayout->addStretch();

    m_topLayout->addWidget(m_imageLabel);
    m_topLayout->addLayout(m_nameAndAuthorLayout, 1);

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

    if (!metadata.image.isEmpty())
        m_imageLabel->setImage(metadata.image, TubeLabel::Cached | TubeLabel::KeepAspectRatio);

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
