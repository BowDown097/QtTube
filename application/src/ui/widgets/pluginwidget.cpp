#include "pluginwidget.h"
#include "ui/widgets/labels/tubelabel.h"
#include "utils/httputils.h"
#include <QBoxLayout>
#include <QDesktopServices>

PluginWidget::PluginWidget(const PluginData* data, QWidget* parent)
    : QWidget(parent),
      authorLabel(new TubeLabel(this)),
      buttonsLayout(new QHBoxLayout),
      descriptionLabel(new TubeLabel(this)),
      imageLabel(new QLabel(this)),
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

    if (data->metadata->image[0] != '\0')
    {
        HttpReply* reply = HttpUtils::cachedInstance().get(QUrl(data->metadata->image));
        connect(reply, &HttpReply::finished, this, &PluginWidget::setImage);
    }

    if (data->metadata->url[0] != '\0')
    {
        nameLabel->setClickable(true);
        nameLabel->setUnderlineOnHover(true);
        connect(nameLabel, &TubeLabel::clicked, std::bind(&QDesktopServices::openUrl, QUrl(data->metadata->url)));
    }
}

void PluginWidget::setImage(const HttpReply& reply)
{
    QPixmap pixmap;
    pixmap.loadFromData(reply.body());
    imageLabel->setPixmap(pixmap.scaled(imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}
