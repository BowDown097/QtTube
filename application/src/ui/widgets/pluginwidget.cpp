#include "pluginwidget.h"
#include "ui/widgets/labels/tubelabel.h"
#include <QBoxLayout>
#include <QDesktopServices>
#include <QPushButton>
#include <QRadioButton>

PluginWidget::PluginWidget(PluginData* data, QWidget* parent)
    : QWidget(parent),
      m_activeButton(new QRadioButton(this)),
      m_authorLabel(new TubeLabel(this)),
      m_buttonsLayout(new QHBoxLayout),
      m_data(data),
      m_descriptionLabel(new TubeLabel(this)),
      m_imageLabel(new TubeLabel(this)),
      m_layout(new QHBoxLayout(this)),
      m_metadataLayout(new QVBoxLayout),
      m_nameLabel(new TubeLabel(this))
{
    m_activeButton->setChecked(data->active);

    m_authorLabel->setElideMode(Qt::ElideRight);
    m_authorLabel->setFont(QFont(m_authorLabel->font().toString(), m_authorLabel->font().pointSize() - 2));
    m_authorLabel->setText(data->metadata->author);

    m_descriptionLabel->setElideMode(Qt::ElideRight);
    m_descriptionLabel->setMaximumLines(2);
    m_descriptionLabel->setWordWrap(true);
    m_descriptionLabel->setText(data->metadata->description);

    m_imageLabel->setFixedSize(48, 48);
    m_imageLabel->setScaledContents(true);

    m_nameLabel->setElideMode(Qt::ElideRight);
    m_nameLabel->setFont(QFont(m_nameLabel->font().toString(), m_nameLabel->font().pointSize(), QFont::Bold));
    m_nameLabel->setText(data->metadata->name);

    m_layout->addWidget(m_activeButton);
    m_layout->addWidget(m_imageLabel);
    m_layout->addLayout(m_metadataLayout);

    m_metadataLayout->addWidget(m_nameLabel);
    m_metadataLayout->addWidget(m_authorLabel);
    m_metadataLayout->addWidget(m_descriptionLabel);
    m_metadataLayout->addLayout(m_buttonsLayout);
    m_metadataLayout->addStretch();

    if (data->metadata->image)
        m_imageLabel->setImage(QUrl(data->metadata->image), TubeLabel::Cached | TubeLabel::KeepAspectRatio);

    if (data->metadata->url)
    {
        m_nameLabel->setClickable(true);
        m_nameLabel->setUnderlineOnHover(true);
        connect(m_nameLabel, &TubeLabel::clicked, std::bind(&QDesktopServices::openUrl, QUrl(data->metadata->url)));
    }

    if (data->settings->window())
    {
        QPushButton* openSettingsButton = new QPushButton("Open settings", this);
        m_buttonsLayout->addWidget(openSettingsButton);
        connect(openSettingsButton, &QPushButton::clicked, this, [data] {
            QWidget* window = data->settings->window();
            window->setAttribute(Qt::WA_DeleteOnClose);
            window->show();
        });
    }

    m_buttonsLayout->addStretch();
}
