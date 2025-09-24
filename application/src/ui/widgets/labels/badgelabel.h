#pragma once
#include "qttube-plugin/objects/badge.h"
#include <QLabel>

class BadgeLabel : public QLabel
{
public:
    explicit BadgeLabel(QWidget* parent = nullptr);
    void setData(const QtTubePlugin::Badge& badge);
protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent*) override;
#else
    void enterEvent(QEvent*) override;
#endif
    void leaveEvent(QEvent*) override;
private:
    QString m_hoveredStylesheet;
    QString m_normalStylesheet;
};
