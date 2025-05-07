#pragma once
#include <QJsonValue>
#include <QWidget>

class QVBoxLayout;
class TubeLabel;

class SpecialMessage : public QWidget
{
public:
    explicit SpecialMessage(const QJsonValue& renderer, QWidget* parent = nullptr,
                            const QString& headerKey = "text", const QString& subtextKey = "subtext",
                            bool subtextItalic = true, const QString& background = "black");
private:
    TubeLabel* header;
    QVBoxLayout* layout;
    TubeLabel* subtext;
};
