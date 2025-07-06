#include "specialmessage.h"
#include "ui/widgets/labels/tubelabel.h"
#include <QBoxLayout>

constexpr QLatin1String Stylesheet(R"(
    background: %1;
    border: 1px solid transparent;
    border-radius: 4px;
    color: white;
)");

SpecialMessage::SpecialMessage(const QtTubePlugin::SpecialMessage& data, QWidget* parent)
    : QWidget(parent),
      header(new TubeLabel(this)),
      layout(new QVBoxLayout(this)),
      subtext(new TubeLabel(this))
{
    setAutoFillBackground(true);
    setStyleSheet(Stylesheet.arg(data.backgroundColor));
    layout->setContentsMargins(0, 0, 0, 0);

    if (!data.header.isEmpty())
    {
        QFont headerFont(font().toString(), -1, QFont::Bold);
        headerFont.setStyle(data.headerStyle);

        header->setAlignment(Qt::AlignCenter);
        header->setFont(headerFont);
        header->setText(data.header);
        header->setWordWrap(true);
        layout->addWidget(header);
    }

    QFont subtextFont(font().toString());
    subtextFont.setStyle(data.contentStyle);

    subtext->setAlignment(Qt::AlignCenter);
    subtext->setFont(subtextFont);
    subtext->setTextFormat(Qt::RichText);
    subtext->setWordWrap(true);
    subtext->setText(data.content, true, TubeLabel::Cached);
    layout->addWidget(subtext);
}
