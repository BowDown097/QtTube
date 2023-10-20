#ifndef CONCLUSIONPAGE_H
#define CONCLUSIONPAGE_H
#include "basicinfopage.h"

constexpr const char* info = "You have completed this wizard. Use the Subscriptions and/or History tab(s) to make sure "
                             "everything was done successfully.";

class ConclusionPage : public BasicInfoPage
{
public:
    explicit ConclusionPage(QWidget* parent = nullptr) : BasicInfoPage("Wizard Complete", info, parent) {}
};

#endif // CONCLUSIONPAGE_H
