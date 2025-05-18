#include "pluginwidget.h"
#include "ui/widgets/labels/tubelabel.h"
#include <QBoxLayout>
#include <QDesktopServices>
#include <QPushButton>

PluginWidget::PluginWidget(const PluginData* data, QWidget* parent)
    : QWidget(parent),
      authorLabel(new TubeLabel(this)),
      buttonsLayout(new QHBoxLayout),
      descriptionLabel(new TubeLabel(this)),
      imageLabel(new TubeLabel(this)),
      layout(new QHBoxLayout(this)),
      metadataLayout(new QVBoxLayout),
      nameLabel(new TubeLabel(this))
{
    authorLabel->setElideMode(Qt::ElideRight);
    authorLabel->setFont(QFont(authorLabel->font().toString(), authorLabel->font().pointSize() - 2));
    authorLabel->setText(data->metadata->author);

    descriptionLabel->setElideMode(Qt::ElideRight);
    descriptionLabel->setMaximumLines(2);
    descriptionLabel->setWordWrap(true);
    descriptionLabel->setText(data->metadata->description);

    imageLabel->setFixedSize(48, 48);
    imageLabel->setScaledContents(true);

    nameLabel->setElideMode(Qt::ElideRight);
    nameLabel->setFont(QFont(nameLabel->font().toString(), nameLabel->font().pointSize(), QFont::Bold));
    nameLabel->setText(data->metadata->name);

    layout->addWidget(imageLabel);
    layout->addLayout(metadataLayout);

    metadataLayout->addWidget(nameLabel);
    metadataLayout->addWidget(authorLabel);
    metadataLayout->addWidget(descriptionLabel);
    metadataLayout->addLayout(buttonsLayout);
    metadataLayout->addStretch();

    if (data->metadata->image)
        imageLabel->setImage(QUrl(data->metadata->image), TubeLabel::Cached | TubeLabel::KeepAspectRatio);

    if (data->metadata->url)
    {
        nameLabel->setClickable(true);
        nameLabel->setUnderlineOnHover(true);
        connect(nameLabel, &TubeLabel::clicked, std::bind(&QDesktopServices::openUrl, QUrl(data->metadata->url)));
    }

    if (data->settings->window())
    {
        QPushButton* openSettingsButton = new QPushButton("Open settings", this);
        buttonsLayout->addWidget(openSettingsButton);
        connect(openSettingsButton, &QPushButton::clicked, this, [data] {
            QWidget* window = data->settings->window();
            window->setAttribute(Qt::WA_DeleteOnClose);
            window->show();
        });
    }

    buttonsLayout->addStretch();
}
