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
      m_headerLabel(new TubeLabel(this)),
      m_layout(new QVBoxLayout(this)),
      m_subtextLabel(new TubeLabel(this))
{
    setAutoFillBackground(true);
    setStyleSheet(Stylesheet.arg(data.backgroundColor));
    m_layout->setContentsMargins(0, 0, 0, 0);

    if (!data.header.isEmpty())
    {
        QFont headerFont(font().toString(), -1, QFont::Bold);
        headerFont.setStyle(data.headerStyle);

        m_headerLabel->setAlignment(Qt::AlignCenter);
        m_headerLabel->setFont(headerFont);
        m_headerLabel->setText(data.header);
        m_headerLabel->setWordWrap(true);
        m_layout->addWidget(m_headerLabel);
    }

    QFont subtextFont(font().toString());
    subtextFont.setStyle(data.contentStyle);

    m_subtextLabel->setAlignment(Qt::AlignCenter);
    m_subtextLabel->setFont(subtextFont);
    m_subtextLabel->setTextFormat(Qt::RichText);
    m_subtextLabel->setWordWrap(true);
    m_subtextLabel->setText(data.content, true, TubeLabel::Cached);
    m_layout->addWidget(m_subtextLabel);
}
