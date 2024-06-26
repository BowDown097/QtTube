#pragma once
#include "basicinfopage.h"

constexpr QLatin1String ConclusionInfo(R"(You have completed this wizard. Use the Subscriptions and/or
History tab(s) to make sure everything was done successfully.)");

class ConclusionPage : public BasicInfoPage
{
public:
    explicit ConclusionPage(QWidget* parent = nullptr) : BasicInfoPage("Wizard Complete", ConclusionInfo, parent) {}
};
