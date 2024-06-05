#include "specialmessage.h"
#include "innertube/objects/innertubestring.h"
#include "ui/widgets/labels/tubelabel.h"
#include <QBoxLayout>

#define STYLE QStringLiteral(R"(
    background: %1;
    border: 1px solid transparent;
    border-radius: 4px;
    color: white;
)")

SpecialMessage::SpecialMessage(const QJsonValue& renderer, QWidget* parent, const QString& headerKey,
                               const QString& subtextKey, bool subtextItalic, const QString& background)
    : QWidget(parent),
      header(new TubeLabel(this)),
      layout(new QVBoxLayout(this)),
      subtext(new TubeLabel(InnertubeObjects::InnertubeString(renderer[subtextKey]), this))
{
    setAutoFillBackground(true);
    setStyleSheet(STYLE.arg(background));
    layout->setContentsMargins(0, 0, 0, 0);

    InnertubeObjects::InnertubeString headerString(renderer[headerKey]);
    if (!headerString.text.isEmpty())
    {
        header->setAlignment(Qt::AlignCenter);
        header->setFont(QFont(font().toString(), -1, QFont::Bold));
        header->setText(headerString.text);
        header->setWordWrap(true);
        layout->addWidget(header);
    }

    subtext->setAlignment(Qt::AlignCenter);
    subtext->setFont(QFont(font().toString(), -1, -1, subtextItalic));
    subtext->setWordWrap(true);
    layout->addWidget(subtext);
}
